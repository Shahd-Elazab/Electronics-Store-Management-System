#include "GuiController.h"
#include <stdexcept>

// ─────────────────────────────────────────────────────────────────────────────
// Catalog
// ─────────────────────────────────────────────────────────────────────────────
void GuiController::updateCatalog() {
    auto items = std::make_shared<slint::VectorModel<ProductItem>>();

    for (auto* p : inventory.all()) {
        ProductItem item;
        item.id      = slint::SharedString(p->getProductId());
        item.name    = slint::SharedString(p->getName());
        item.brand   = slint::SharedString(p->getBrand());
        item.price   = slint::SharedString(fmtPrice(p->getCurrentPrice()));
        item.stock   = slint::SharedString(std::to_string(p->getStockQuantity()));
        item.status  = slint::SharedString(p->getStatusString());
        item.on_sale = p->getOnSale();

        // getType() returns lowercase: "smartphone", "laptop", "accessory"
        std::string t = p->getType();
        if      (t == "smartphone") item.type = slint::SharedString("Smartphone");
        else if (t == "laptop")     item.type = slint::SharedString("Laptop");
        else                        item.type = slint::SharedString("Accessory");

        items->push_back(item);
    }
    ui->set_catalog_items(items);
}

// ─────────────────────────────────────────────────────────────────────────────
// Cart  (Store.h CartItem has fields: item = Product*, quantity = int)
// ─────────────────────────────────────────────────────────────────────────────
void GuiController::updateCart() {
    if (!currentUser || currentUser->getRole() != 'C') return;

    Customer* cust = dynamic_cast<Customer*>(currentUser);
    if (!cust) return;

    auto items = std::make_shared<slint::VectorModel<UiCartItem>>();
    double total = 0.0;

    for (const auto& ci : cust->getCart().getItems()) {
        UiCartItem ui_item;
        ui_item.product_id = slint::SharedString(ci.item->getProductId());
        ui_item.name       = slint::SharedString(ci.item->getName());
        ui_item.price      = slint::SharedString(fmtPrice(ci.item->getCurrentPrice()));
        ui_item.quantity   = slint::SharedString(std::to_string(ci.quantity));
        double sub         = ci.item->getCurrentPrice() * ci.quantity;
        ui_item.subtotal   = slint::SharedString(fmtPrice(sub));
        total             += sub;
        items->push_back(ui_item);
    }

    ui->set_cart_items(items);
    ui->set_cart_total(slint::SharedString(fmtPrice(total)));
}

// ─────────────────────────────────────────────────────────────────────────────
// Orders
// ─────────────────────────────────────────────────────────────────────────────
void GuiController::updateOrders() {
    if (!currentUser) return;
    std::string custId = std::to_string(currentUser->getID());

    auto items = std::make_shared<slint::VectorModel<OrderItem>>();
    for (const auto& o : orders) {
        if (!o.isValid()) continue;
        if (currentUser->getRole() == 'C' && o.getCustomerID() != custId) continue;

        OrderItem oi;
        oi.order_id = slint::SharedString(std::to_string(o.getOrderID()));
        oi.date     = slint::SharedString(o.getOrderDate());
        oi.total    = slint::SharedString(fmtPrice(o.getTotalAmount()));
        items->push_back(oi);
    }
    ui->set_order_items(items);
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor — wire all callbacks
// ─────────────────────────────────────────────────────────────────────────────
GuiController::GuiController()
    : um(userManager::getUMinstance()), fm(inventory, *um, orders)
{
    fm.loadAll();

    ui->set_user_role("None");
    ui->set_active_page("catalog");

    // ── Login ─────────────────────────────────────────────────────────────
    ui->on_login_requested([this](slint::SharedString id_str, slint::SharedString pass) {
        try {
            int id = std::stoi(std::string(id_str));
            for (auto* user : um->getUsers()) {
                if (user->getID() == id && user->checkPassword(std::string(pass))) {
                    currentUser = user;
                    ui->set_user_name(slint::SharedString(user->getName()));
                    ui->set_user_role(user->getRole() == 'A'
                        ? slint::SharedString("Admin")
                        : slint::SharedString("Customer"));
                    ui->set_status_message("");
                    ui->set_active_page("catalog");
                    updateCatalog();
                    return;
                }
            }
            ui->set_status_message("Invalid ID or password.");
        } catch (...) {
            ui->set_status_message("Please enter a numeric ID.");
        }
    });

    // ── Sign Up ───────────────────────────────────────────────────────────
    ui->on_signup_requested([this](
        slint::SharedString name,  slint::SharedString email,
        slint::SharedString pass,  slint::SharedString phone,
        slint::SharedString addr)
    {
        std::string n  = std::string(name);
        std::string e  = std::string(email);
        std::string p  = std::string(pass);
        std::string ph = std::string(phone);
        std::string a  = std::string(addr);

        if (n.empty() || e.empty() || p.empty()) {
            ui->set_status_message("Name, email and password are required.");
            return;
        }
        User* u = new Customer(n, e, p, ph, a);
        FILE* fptr = fopen("users.csv", "a");
        if (fptr) {
            fprintf(fptr, "%d,%s,%s,%s,%s,%s,C\n",
                u->getID(), n.c_str(), e.c_str(),
                u->getPasswordHash().c_str(), ph.c_str(), a.c_str());
            fclose(fptr);
        }
        std::string msg = "Account created! Your ID is " + std::to_string(u->getID());
        delete u;
        fm.loadUsers();
        ui->set_status_message(slint::SharedString(msg));
    });

    // ── Logout ────────────────────────────────────────────────────────────
    ui->on_logout_requested([this]() {
        currentUser = nullptr;
        ui->set_user_role("None");
        ui->set_user_name("Guest");
        ui->set_status_message("");
        ui->set_active_page("catalog");
        ui->set_catalog_items(std::make_shared<slint::VectorModel<ProductItem>>());
        ui->set_cart_items(std::make_shared<slint::VectorModel<UiCartItem>>());
        ui->set_order_items(std::make_shared<slint::VectorModel<OrderItem>>());
        ui->set_cart_total("0.00");
    });

    // ── Navigate ──────────────────────────────────────────────────────────
    ui->on_navigate([this](slint::SharedString page) {
        std::string p = std::string(page);
        ui->set_active_page(slint::SharedString(p));
        ui->set_status_message("");
        if (p == "catalog") updateCatalog();
        if (p == "cart")    updateCart();
        if (p == "orders")  updateOrders();
    });

    // ── Search ────────────────────────────────────────────────────────────
    ui->on_search_requested([this](slint::SharedString query) {
        std::string q = std::string(query);
        auto items = std::make_shared<slint::VectorModel<ProductItem>>();

        std::vector<Product*> matches = q.empty()
            ? inventory.all()
            : inventory.findByName(q);

        for (auto* p : matches) {
            ProductItem item;
            item.id      = slint::SharedString(p->getProductId());
            item.name    = slint::SharedString(p->getName());
            item.brand   = slint::SharedString(p->getBrand());
            item.price   = slint::SharedString(fmtPrice(p->getCurrentPrice()));
            item.stock   = slint::SharedString(std::to_string(p->getStockQuantity()));
            item.status  = slint::SharedString(p->getStatusString());
            item.on_sale = p->getOnSale();

            // lowercase match
            std::string t = p->getType();
            if      (t == "smartphone") item.type = slint::SharedString("Smartphone");
            else if (t == "laptop")     item.type = slint::SharedString("Laptop");
            else                        item.type = slint::SharedString("Accessory");

            items->push_back(item);
        }
        ui->set_catalog_items(items);
    });

    // ── Add to Cart ───────────────────────────────────────────────────────
    ui->on_add_to_cart([this](slint::SharedString pid, int qty) {
        if (!currentUser || currentUser->getRole() != 'C') return;
        Product* p = inventory.findById(std::string(pid));
        if (!p)               { ui->set_status_message("Product not found.");        return; }
        if (!p->isAvailable()) { ui->set_status_message("Product not available.");   return; }
        currentUser->addToCart(p, qty > 0 ? qty : 1);
        ui->set_status_message(slint::SharedString(p->getName() + " added to cart."));
        updateCart();
    });

    // ── Remove from Cart ──────────────────────────────────────────────────
    ui->on_remove_from_cart([this](slint::SharedString pid) {
        if (!currentUser) return;
        currentUser->removeFromCart(std::string(pid));
        ui->set_status_message("Item removed.");
        updateCart();
    });

    // ── Checkout ──────────────────────────────────────────────────────────
    ui->on_checkout_requested([this]() {
        if (!currentUser || currentUser->getRole() != 'C') return;
        Order receipt = currentUser->checkout(inventory);
        if (receipt.isValid()) {
            orders.push_back(receipt);
            inventory.saveToFile();
            fm.saveOrders();
            ui->set_status_message(slint::SharedString(
                "Order #" + std::to_string(receipt.getOrderID()) +
                " placed! Total: $" + fmtPrice(receipt.getTotalAmount())));
            updateCart();
            updateOrders();
        } else {
            ui->set_status_message("Checkout failed. Cart may be empty.");
        }
    });

    // ── Delete Product (Admin) ────────────────────────────────────────────
    ui->on_delete_product([this](slint::SharedString pid) {
        if (!currentUser || currentUser->getRole() != 'A') return;
        if (inventory.removeProduct(std::string(pid))) {
            ui->set_status_message("Product deleted.");
            inventory.saveToFile();
        } else {
            ui->set_status_message("Delete failed.");
        }
        updateCatalog();
    });

    // ── Restock Product (Admin) — qty entered by user in the card ─────────
    ui->on_restock_product_qty([this](slint::SharedString pid, slint::SharedString qty_str) {
        if (!currentUser || currentUser->getRole() != 'A') return;
        int qty = 0;
        try { qty = std::stoi(std::string(qty_str)); } catch (...) {}
        if (qty <= 0) {
            ui->set_status_message("Enter a valid restock quantity.");
            return;
        }
        if (inventory.restock(std::string(pid), qty)) {
            ui->set_status_message(slint::SharedString(
                "Restocked " + std::string(pid) + " with " + std::to_string(qty) + " units."));
            inventory.saveToFile();
        } else {
            ui->set_status_message("Restock failed.");
        }
        updateCatalog();
    });

    // ── Discontinue Product (Admin) ───────────────────────────────────────
    ui->on_discontinue_product([this](slint::SharedString pid) {
        if (!currentUser || currentUser->getRole() != 'A') return;
        try {
            if (inventory.discontinue(std::string(pid))) {
                ui->set_status_message("Product discontinued.");
                inventory.saveToFile();
            } else {
                ui->set_status_message("Operation failed.");
            }
        } catch (const std::exception& ex) {
            ui->set_status_message(slint::SharedString(ex.what()));
        }
        updateCatalog();
    });

    // ── Add Product (Admin) ───────────────────────────────────────────────
    ui->on_add_product_requested([this](
        slint::SharedString pid,   slint::SharedString name,
        slint::SharedString brand, slint::SharedString model,
        slint::SharedString price_str, slint::SharedString stock_str,
        slint::SharedString type_str)
    {
        if (!currentUser || currentUser->getRole() != 'A') return;

        std::string id = std::string(pid);
        std::string nm = std::string(name);
        std::string br = std::string(brand);
        std::string md = std::string(model);
        std::string tp = std::string(type_str);

        if (id.empty() || nm.empty()) {
            ui->set_status_message("ID and Name are required.");
            return;
        }

        double price = 0.0;
        int    stock = 0;
        try { price = std::stod(std::string(price_str)); } catch (...) {}
        try { stock = std::stoi(std::string(stock_str)); } catch (...) {}

        Product* pro = nullptr;
        try {
            if (tp == "smartphone") {
                pro = ProductFactory::createSmartPhone(
                    id, nm, br, md, price, stock,
                    4, 128, 4000, 6.5, 12, "Android");
            } else if (tp == "laptop") {
                pro = ProductFactory::createLaptop(
                    id, nm, br, md, price, stock,
                    8, 512, true, 15.6, "Windows 11",
                    "Unknown CPU", "Unknown GPU", 50,
                    LaptopFormFactor::ULTRABOOK);
            } else {
                pro = ProductFactory::createAccessory(
                    id, nm, br, md, price, stock,
                    AccessoryCategory::CASE_COVER);
            }

            if (inventory.addProduct(pro)) {
                inventory.saveToFile();
                ui->set_status_message(slint::SharedString(nm + " added successfully."));
                updateCatalog();
            } else {
                delete pro;
                ui->set_status_message("Failed: ID already exists.");
            }
        } catch (const std::exception& ex) {
            ui->set_status_message(slint::SharedString(ex.what()));
        }
    });

    // ── Apply Discount (Admin) ────────────────────────────────────────────
    ui->on_apply_discount_requested([this](
        slint::SharedString pid, float amount, slint::SharedString dtype)
    {
        if (!currentUser || currentUser->getRole() != 'A') return;
        DiscountType dt = (std::string(dtype) == "percentage")
            ? DiscountType::PERCENTAGE
            : DiscountType::FIXED_AMOUNT;
        try {
            if (inventory.applyDiscount(std::string(pid), (double)amount, dt)) {
                inventory.saveToFile();
                ui->set_status_message("Discount applied.");
                updateCatalog();
            } else {
                ui->set_status_message("Discount failed.");
            }
        } catch (const std::exception& ex) {
            ui->set_status_message(slint::SharedString(ex.what()));
        }
    });
}

// ─────────────────────────────────────────────────────────────────────────────
void GuiController::run() {
    ui->run();
    fm.saveAll();
}