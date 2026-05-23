/*******************************************************************************
 * FILE: Store.cpp
 *
 * ROLE:
 * This is the single implementation file for the entire store system
 * It contains all method definitions for every class declared in Store.h
 *
 * CONTENTS:
 *   1. Product hierarchy (Product, SmartPhone, Laptop, Accessory)
 *   2. Cart
 *   3. Order
 *   4. Inventory
 *   5. ProductFactory
 *   6. User hierarchy (User, Customer, Admin, userManager)
 *   7. FileManager
 *******************************************************************************/

#include "Store.h"

// =============================================================================
// Static / global variable definitions used in the file
// =============================================================================

int maxID = 100000; // starting point for user ids
int Order::order_count = 0;
int User::user_count  = maxID + 1;
userManager* userManager::UMptr = nullptr;

// =============================================================================
// Product
// =============================================================================

// DESIGN PATTERNS:
// - Inheritance: Utilizes a public inheritance tree to minimize code duplication
// - Polymorphism: Implements virtual functions (displayDetails, getType)
//   allowing the Store Management system to process different product types
//   through a single interface (Product*)
// - Encapsulation: Protects sensitive data (like pricing and stock) through
//   private members and controlled setters/getters with validation logic

// =============================================================================
// Product  —  private helper functions
// =============================================================================

void Product::updateStatus() {
    if (status == ProductStatus::DISCONTINUED || status == ProductStatus::COMING_SOON)
        return;
    status = (stockQuantity > 0)? ProductStatus::AVAILABLE:ProductStatus::OUT_OF_STOCK;
}

// matches enum class status to its appropriate string
// no breaks needed as we return a value
string Product::statusToString() const {
    switch (status) {
        case ProductStatus::AVAILABLE:
            return "Available";
        case ProductStatus::OUT_OF_STOCK:
            return "Out of Stock";
        case ProductStatus::DISCONTINUED:
            return "Discontinued";
        case ProductStatus::COMING_SOON:
            return "Coming Soon";
        default:
            return "Unknown";
    }
}

// =============================================================================
// Product  —  protected
// =============================================================================

void Product::displayBase(ostream &out) const {
    out << "========================================" << endl;
    out << "Name  : " << name << " (" << brand << " " << model << ")" << endl;
    out << "SKU   : " << sku << " | ID: " << productId << endl;
    out << "Price : $" << fixed << setprecision(2) << currentPrice;
    if (onSale)
        out << " [ON SALE - was $" << fixed << setprecision(2) << mainPrice << "]";
    out << endl;
    out << "Stock : " << stockQuantity << " | Status: " << statusToString() << endl;
    if (!description.empty())
        out << "Desc  : " << description << endl;
}

Product::Product() : mainPrice(0.0), currentPrice(0.0), discount(0.0), onSale(false),
                     status(ProductStatus::OUT_OF_STOCK), stockQuantity(0) {}

// =============================================================================
// Product  —  public
// =============================================================================

Product::Product(const string &id, const string &sku,
                 const string &name, const string &brand,
                 const string &model, double price, int stock)
    : productId(id), sku(sku), name(name), brand(brand), model(model),
      mainPrice(price), currentPrice(price), discount(0.0), onSale(false),
      status(stock > 0 ? ProductStatus::AVAILABLE : ProductStatus::OUT_OF_STOCK),
      stockQuantity(stock) {
    if (price < 0)
        throw invalid_argument("Price cannot be negative.");
    if (stock < 0)
        throw invalid_argument("Stock cannot be negative.");
}

Product::~Product() {}

void Product::print(ostream &out) const { displayBase(out); }

// Getters
const string &Product::getProductId() const { return productId; }
const string &Product::getSku() const { return sku; }
const string &Product::getName() const { return name; }
const string &Product::getBrand() const { return brand; }
const string &Product::getModel() const { return model; }
const string &Product::getDescription() const { return description; }
double Product::getMainPrice() const { return mainPrice; }
double Product::getCurrentPrice() const { return currentPrice; }
double Product::getDiscount() const { return discount; }
bool Product::getOnSale() const { return onSale; }
int Product::getStockQuantity() const { return stockQuantity; }
ProductStatus Product::getStatus() const { return status; }
string Product::getStatusString() const { return statusToString(); }

// Setters / operations
void Product::setDescription(const string &d) { description = d; }

void Product::setDiscountSale(double amount, DiscountType type) {
    if (amount == 0.0) {
        resetDiscount();
        return;
    }

    if (type == DiscountType::PERCENTAGE) {
        if (amount < 0 || amount > 100)
            throw invalid_argument("Percentage must be between 0 and 100.");
        discount = mainPrice * (amount / 100.0);
    }
    else {
        if (amount < 0 || amount > mainPrice)
            throw invalid_argument("Fixed discount must be between 0 and the product price.");
        discount = amount;
    }
    currentPrice = mainPrice - discount;
    onSale = true;
}

void Product::resetDiscount() {
    discount = 0.0;
    currentPrice = mainPrice;
    onSale = false;
}

void Product::markComingSoon() {
    status = ProductStatus::COMING_SOON;
    stockQuantity = 0;
}

void Product::discontinue() {
    status = ProductStatus::DISCONTINUED;
    stockQuantity = 0;
}

double Product::getFinalPrice(double taxRate) const {
    if (taxRate < 0)
        throw invalid_argument("Tax rate cannot be negative.");
    return currentPrice * (1.0 + taxRate / 100.0);
}

bool Product::sell(int quantity) {
    if (quantity <= 0)
        throw invalid_argument("Quantity to sell must be positive.");
    if (status == ProductStatus::DISCONTINUED)
        throw logic_error("Cannot sell a discontinued product.");
    if (status == ProductStatus::COMING_SOON)
        throw logic_error("Cannot sell a product that is not yet available.");
    if (stockQuantity < quantity) {
        cerr << "Warning: insufficient stock for '" << name << "' (requested " <<
                 quantity << ", available " << stockQuantity << ")." << endl;
        return false;
    }
    stockQuantity -= quantity;
    updateStatus();
    return true;
}

void Product::restock(int quantity) {
    if (quantity <= 0)
        throw invalid_argument("Quantity to restock must be positive.");
    if (status == ProductStatus::DISCONTINUED)
        throw logic_error("Cannot restock a discontinued product.");
    stockQuantity += quantity;
    updateStatus();
}

bool Product::isAvailable() const {
    return status == ProductStatus::AVAILABLE && stockQuantity > 0;
}

// operator overloading
bool Product::operator==(const Product &other) const { return productId == other.productId; }
bool Product::operator<(const Product &other) const { return currentPrice < other.currentPrice; }

ostream &operator<<(ostream &out, const Product &p) {
    p.print(out);
    return out;
}

// =============================================================================
// SmartPhone
// =============================================================================

SmartPhone::SmartPhone(const string &id, const string &sku,
                       const string &name, const string &brand,
                       const string &model, double price, int stock,
                       int ram, int storage, int battery,
                       double display, int camera, const string &os)
    : Product(id, sku, name, brand, model, price, stock),
      ram(ram), storage(storage), batteryMah(battery),
      inches(display), cameraMP(camera), phoneOS(os),
      has5G(false), hasNFC(false), refreshRateHz(60), chipset("") {
    if (ram <= 0)
        throw invalid_argument("RAM must be positive.");
    if (storage <= 0)
        throw invalid_argument("Storage must be positive.");
    if (battery <= 0)
        throw invalid_argument("Battery must be positive.");
    if (display <= 0)
        throw invalid_argument("Display size must be positive.");
    if (camera <= 0)
        throw invalid_argument("Camera MP must be positive.");
}

// Setters
void SmartPhone::setHas5G(bool v) { has5G = v; }
void SmartPhone::setHasNFC(bool v) { hasNFC = v; }
void SmartPhone::setChipset(const string &c) { chipset = c; }
void SmartPhone::setOS(const string &os) { phoneOS = os; }
void SmartPhone::setRefreshRate(int hz) {
    if (hz <= 0)
        throw invalid_argument("Refresh rate must be positive.");
    refreshRateHz = hz;
}
void SmartPhone::setRam(int r) {
    if (r <= 0)
        throw invalid_argument("RAM must be positive.");
    ram = r;
}
void SmartPhone::setStorage(int s) {
    if (s <= 0)
        throw invalid_argument("Storage must be positive.");
    storage = s;
}
void SmartPhone::setBattery(int mah) {
    if (mah <= 0)
        throw invalid_argument("Battery must be positive.");
    batteryMah = mah;
}
void SmartPhone::setDisplay(double d) {
    if (d <= 0)
        throw invalid_argument("Display size must be positive.");
    inches = d;
}
void SmartPhone::setCamera(int mp) {
    if (mp <= 0)
        throw invalid_argument("Camera MP must be positive.");
    cameraMP = mp;
}

// Getters
int SmartPhone::getRam() const { return ram; }
int SmartPhone::getStorage() const { return storage; }
int SmartPhone::getBattery() const { return batteryMah; }
double SmartPhone::getDisplay() const { return inches; }
int SmartPhone::getCamera() const { return cameraMP; }
const string &SmartPhone::getOS() const { return phoneOS; }
bool SmartPhone::getHas5G() const { return has5G; }
bool SmartPhone::getHasNFC() const { return hasNFC; }
int SmartPhone::getRefreshRate() const { return refreshRateHz; }
const string &SmartPhone::getChipset() const { return chipset; }

bool SmartPhone::isFlagship() const { return ram >= 12 && cameraMP >= 100; }
bool SmartPhone::isGamingPhone()const { return ram >= 8 && batteryMah >= 4500; }
bool SmartPhone::isMidRange() const { return getCurrentPrice() >= 300.0 && getCurrentPrice() < 700.0; }
double SmartPhone::estimatedBatteryDays(int mAhPerDay) const {
    if (mAhPerDay <= 0)
        throw invalid_argument("Usage must be positive.");
    return static_cast<double>(batteryMah) / mAhPerDay;
}

string SmartPhone::getType() const { return "smartphone"; }
void SmartPhone::displayDetails() const { cout << *this; }
void SmartPhone::print(ostream &out) const {
    displayBase(out);
    out << "--- Smartphone Details ---" << endl;
    if (!chipset.empty())
        out << "Chipset     : " << chipset << endl;
    out << "RAM         : " << ram        << " GB" << endl;
    out << "Storage     : " << storage    << " GB" << endl;
    out << "Battery     : " << batteryMah << " mAh" << endl;
    out << "Display     : " << inches     << "\" @ " << refreshRateHz << " Hz" << endl;
    out << "Camera      : " << cameraMP   << " MP" << endl;
    out << "OS          : " << phoneOS    << endl;
    out << "5G          : " << (has5G     ? "Yes" : "No") << endl;
    out << "NFC         : " << (hasNFC    ? "Yes" : "No") << endl;
    out << "Flagship    : " << (isFlagship()    ? "Yes" : "No") << endl;
    out << "Gaming      : " << (isGamingPhone() ? "Yes" : "No") << endl;
    out << "========================================" << endl;
}

// =============================================================================
// Laptop  —  private helper
// =============================================================================

string Laptop::formFactorToString() const {
    switch (formFactor) {
        case LaptopFormFactor::ULTRABOOK:
            return "Ultrabook";
        case LaptopFormFactor::GAMING:
            return "Gaming";
        case LaptopFormFactor::WORKSTATION:
            return "Workstation";
        case LaptopFormFactor::CONVERTIBLE:
            return "Convertible (2-in-1)";
        case LaptopFormFactor::CHROMEBOOK:
            return "Chromebook";
        default:
            return "Unknown";
    }
}

// =============================================================================
// Laptop  —  public
// =============================================================================

Laptop::Laptop(const string &id, const string &sku,
               const string &name, const string &brand,
               const string &model, double price, int stock,
               int ram, int storage, bool ssd,
               double display, const string &os,
               const string &cpu, const string &gpu,
               int batteryWh, LaptopFormFactor form,
               double weightKg)
    : Product(id, sku, name, brand, model, price, stock),
      ramGB(ram), storageGB(storage), isSSD(ssd),
      displayInches(display), operatingSystem(os),
      cpu(cpu), gpu(gpu), batteryWh(batteryWh), formFactor(form),
      hasTouchscreen(false), usbPorts(0), hasThunderbolt(false),
      weightKg(weightKg) {
    if (ram <= 0)
        throw invalid_argument("RAM must be positive.");
    if (storage <= 0)
        throw invalid_argument("Storage must be positive.");
    if (display <= 0)
        throw invalid_argument("Display size must be positive.");
    if (batteryWh <= 0)
        throw invalid_argument("Battery capacity must be positive.");
    if (weightKg < 0)
        throw invalid_argument("Weight cannot be negative.");
}

// Setters
void Laptop::setTouchscreen(bool v) { hasTouchscreen = v; }
void Laptop::setThunderbolt(bool v) { hasThunderbolt = v; }
void Laptop::setFormFactor(LaptopFormFactor f) { formFactor = f; }
void Laptop::setSSD(bool v) { isSSD = v; }
void Laptop::setOS(const string &os) { operatingSystem = os; }
void Laptop::setCPU(const string &c) { cpu = c; }
void Laptop::setGPU(const string &g) { gpu = g; }
void Laptop::setUsbPorts(int n) {
    if (n < 0)
        throw invalid_argument("USB port count cannot be negative.");
    usbPorts = n;
}
void Laptop::setRam(int r) {
    if (r <= 0)
        throw invalid_argument("RAM must be positive.");
    ramGB = r;
}
void Laptop::setStorage(int s) {
    if (s <= 0)
        throw invalid_argument("Storage must be positive.");
    storageGB = s;
}
void Laptop::setDisplay(double d) {
    if (d <= 0)
        throw invalid_argument("Display size must be positive.");
    displayInches = d;
}
void Laptop::setBatteryWh(int wh) {
    if (wh <= 0)
        throw invalid_argument("Battery capacity must be positive.");
    batteryWh = wh;
}
void Laptop::setWeight(double kg) {
    if (kg < 0)
        throw invalid_argument("Weight cannot be negative.");
    weightKg = kg;
}

// Getters
int Laptop::getRam() const { return ramGB; }
int Laptop::getStorage() const { return storageGB; }
bool Laptop::getSSD() const { return isSSD; }
double Laptop::getDisplay() const { return displayInches; }
const string &Laptop::getOS() const { return operatingSystem; }
const string &Laptop::getCPU() const { return cpu; }
const string &Laptop::getGPU() const { return gpu; }
int Laptop::getBatteryWh() const { return batteryWh; }
LaptopFormFactor Laptop::getFormFactor() const { return formFactor; }
bool Laptop::getTouchscreen() const { return hasTouchscreen; }
int Laptop::getUsbPorts() const { return usbPorts; }
bool Laptop::getThunderbolt() const { return hasThunderbolt; }
double Laptop::getWeightKg() const { return weightKg; }

bool Laptop::hasDiscreteGPU() const { return !gpu.empty(); }
bool Laptop::isWorkhorseReady() const { return ramGB >= 16 && isSSD && storageGB >= 512; }
bool Laptop::isUltrathin() const { return weightKg > 0 && weightKg < 1.5; }
double Laptop::estimatedBatteryHours(int avgWatts) const {
    if (avgWatts <= 0)
        throw invalid_argument("Average watts must be positive.");
    return static_cast<double>(batteryWh) / avgWatts;
}

string Laptop::getType() const { return "laptop"; }
void Laptop::displayDetails() const { cout << *this; }
void Laptop::print(ostream &out) const {
    displayBase(out);
    out << "--- Laptop Details ---" << endl;
    out << "Form Factor : " << formFactorToString() << endl;
    out << "CPU         : " << cpu << endl;
    out << "GPU         : " << (hasDiscreteGPU() ?gpu : "Integrated") << endl;
    out << "RAM         : " << ramGB       << " GB" << endl;
    out << "Storage     : " << storageGB   << " GB " << (isSSD ? "(SSD)" : "(HDD)") << endl;
    out << "Display     : " << displayInches << "\"\n";
    out << "Battery     : " << batteryWh   << " Wh" << endl;
    out << "OS          : " << operatingSystem << endl;
    out << "Touchscreen : " << (hasTouchscreen  ? "Yes" : "No") << endl;
    out << "USB Ports   : " << usbPorts    << endl;
    out << "Thunderbolt : " << (hasThunderbolt   ? "Yes" : "No") << endl;
    out << "Weight      : " << weightKg    << " kg" << endl;
    out << "Workhorse   : " << (isWorkhorseReady()? "Yes" : "No") << endl;
    out << "========================================" << endl;
}

// =============================================================================
// Accessory  —  private helper
// =============================================================================

string Accessory::categoryToString() const {
    switch (category) {
        case AccessoryCategory::CASE_COVER:
            return "Case / Cover";
        case AccessoryCategory::CHARGER:
            return "Charger";
        case AccessoryCategory::CABLE:
            return "Cable";
        case AccessoryCategory::HEADPHONES:
            return "Headphones";
        case AccessoryCategory::SCREEN_PROTECTOR:
            return "Screen Protector";
        case AccessoryCategory::KEYBOARD:
            return "Keyboard";
        case AccessoryCategory::MOUSE:
            return "Mouse";
        default:
            return "Other";
    }
}

// =============================================================================
// Accessory  —  public
// =============================================================================

Accessory::Accessory(const string &id, const string &sku,
                     const string &name, const string &brand,
                     const string &model, double price, int stock,
                     AccessoryCategory cat,
                     const string &color, const string &material)
    : Product(id, sku, name, brand, model, price, stock),
      category(cat), color(color), material(material),
      connectivity(""), isWireless(false) {}

void Accessory::addCompatibility(const string &device) {
    if (!isCompatibleWith(device))
        compatibleWith.push_back(device);
}
void Accessory::removeCompatibility(const string &device) {
    compatibleWith.erase(
        remove(compatibleWith.begin(), compatibleWith.end(), device),
        compatibleWith.end());
}
void Accessory::clearCompatibility() { compatibleWith.clear(); }
bool Accessory::isCompatibleWith(const string &device) const {
    return find(compatibleWith.begin(), compatibleWith.end(), device) != compatibleWith.end();
}
const vector<string> &Accessory::getCompatibleDevices() const { return compatibleWith; }
int Accessory::getCompatibilityCount() const { return static_cast<int>(compatibleWith.size()); }

// Setters
void Accessory::setConnectivity(const string &c) { connectivity = c; }
void Accessory::setWireless(bool v) { isWireless = v; }
void Accessory::setColor(const string &c) { color = c; }
void Accessory::setMaterial(const string &m) { material = m; }
void Accessory::setCategory(AccessoryCategory c) { category = c; }

// Getters
AccessoryCategory Accessory::getCategory() const { return category; }
const string &Accessory::getColor() const { return color; }
const string &Accessory::getMaterial() const { return material; }
const string &Accessory::getConnectivity() const { return connectivity; }
bool Accessory::getIsWireless() const { return isWireless; }

string Accessory::getType() const { return "accessory"; }
void Accessory::displayDetails() const { cout << *this; }
void Accessory::print(ostream &out) const {
    displayBase(out);
    out << "--- Accessory Details ---" << endl;
    out << "Category    : " << categoryToString() << endl;
    if (!color.empty())
        out << "Color       : " << color << endl;
    if (!material.empty())
        out << "Material    : " << material << endl;
    if (!connectivity.empty())
        out << "Connectivity: " << connectivity<< endl;
    out << "Wireless    : " << (isWireless ? "Yes" : "No") << endl;
    if (!compatibleWith.empty()) {
        out << "Compatible  : ";
        for (size_t i = 0; i < compatibleWith.size(); ++i) {
            if (i) out << ", ";
            out << compatibleWith[i];
        }
        out << endl;
    }
    out << "========================================" << endl;
}

// =============================================================================
// Cart
// =============================================================================

Cart::Cart() : totalPrice(0.0) {}

void Cart::addItem(Product* p, int qnty) {
    if (!p) {
        cout << "Cannot add an empty product to the cart" << endl;
        return;
    }
    if (qnty <= 0) {
        cout << "Quantity must be positive" << endl;
        return;
    }
    if (p->getStockQuantity() < qnty) {  // you cannot add qnty from a product if it cannot satisfy it in the first place
        cout << "Insufficient stock for [" << p->getName() << "], Available: " << p->getStockQuantity() << endl;
        return;
    }

    // Check if this product ID already exists in the cart
    for (CartItem& c_item : items) {
        if (c_item.item->getProductId() == p->getProductId()) {
            c_item.quantity += qnty;
            totalPrice += p->getCurrentPrice() * qnty;
            cout << "Updated [" << p->getName() << "] quantity to " << c_item.quantity << endl;
            return;
        }
    }

    // If product is not yet in cart push a new entry
    items.push_back({p, qnty});
    totalPrice += p->getCurrentPrice() * qnty;
    cout << "Added " << qnty << " of [" << p->getName() << "](unit price $"
         << fixed << setprecision(2) << p->getCurrentPrice() // prevent scientific representation
         << ") to cart" << endl;
}

// Removes the CartItem whose product ID matches productId
// Does NOT DELETE the Product object as the ownership stays with Inventory
void Cart::removeItem(const string& productId) {
    for (auto it = items.begin(); it != items.end(); ++it) {
        if (it->item->getProductId() == productId) {
            totalPrice -= it->item->getCurrentPrice() * it->quantity;
            cout << "Removed [" << it->item->getName() << "](qnty " << it->quantity << ") from cart" << endl;
            items.erase(it);
            return;
        }
    }
    cout << "Product ID [" << productId << "] not found in cart" << endl;
}

// Prints a formatted table: Product Name , Qnty , Unit Price , Subtotal
// Accumulates a Grand Total at the bottom
// const function does not modify any state
void Cart::viewCart() const {
    if (items.empty()) {
        cout << "Your cart is empty" << endl;
        return;
    }

    // Table header
    cout << endl << "========== CART ==========" << endl;
    cout << left  << setw(32) << "Product Name" // left alignment
         << right << setw(6)  << "Qnty"  // right alignment
         << right << setw(12) << "Unit Price"  // right alignment
         << right << setw(12) << "Subtotal" << endl;  // right alignment
    cout << string(62, '-') << endl;

    // loop — price × quantity = subtotal per item
    double grandTotal = 0.0;
    for (const CartItem& ci : items) {
        double subtotal = ci.item->getCurrentPrice() * ci.quantity;
        grandTotal += subtotal;

        cout << left  << setw(32) << ci.item->getName()
             << right << setw(6)  << ci.quantity
             << right << setw(11) << fixed << setprecision(2) << ci.item->getCurrentPrice()
             << right << setw(12) << fixed << setprecision(2) << subtotal << endl;
    }

    // Total
    cout << string(62, '-') << endl;
    cout << right << setw(50) << "Grand Total: $"
         << setw(12) << fixed << setprecision(2) << grandTotal << endl;
    cout << "==========================" << endl;
}

// true if the cart holds no items
// a safety gate in viewCart() and checkout()
bool Cart::isEmpty() const {
    return items.empty();
}

// Read only access to the CartItem list of items (Product* + quantity)
// Returns a const reference to avoid over the head copies
const vector<CartItem>& Cart::getItems() const { return items; }

// getter
double Cart::getTotal() const { return totalPrice; }

// Empties the cart after a successful checkout
// Clears only the CartItem structs
// DOES NOT DELETE the Product objects, they belong to the Inventory
void Cart::clear() {
    items.clear();
    totalPrice = 0.0;
    cout << "Cart cleared" << endl;
}

// =============================================================================
// Order  —  private helper
// =============================================================================

// Returns today's date as a YYYY-MM-DD string, not in freedom format
string Order::currentDate() {
    time_t now = time(nullptr);
    tm* ltm = localtime(&now);
    ostringstream oss;
    oss << (1900 + ltm->tm_year)
        << "-" << setw(2) << setfill('0') << (1 + ltm->tm_mon)
        << "-" << setw(2) << setfill('0') << ltm->tm_mday;
    return oss.str();
}

// =============================================================================
// Order  —  public
// =============================================================================

// Called on a successful checkout: Order newOrder(customerID, grandTotal)
// increments the static counter to assign a unique orderID
Order::Order(const string& customerId, double grandTotal) : customerID(customerId), totalAmount(grandTotal), orderDate(currentDate()) {
    orderID = ++order_count;
    cout << "Order #" << orderID << " placed successfully" << endl;
}

// Called when checkout fails like in empty cart case: return Order()
// Sets orderID to -1 to detect an invalid transaction
Order::Order() : orderID(-1), customerID(""), totalAmount(0.0), orderDate("") {}

// Used by FileManager::loadOrders() to reconstruct a saved Order from its persisted fields without incrementing the global counter
// The loaded orderID is applied directly and count is updated
Order::Order(int restoredId, const string& cId, double total, const string& date) : orderID(restoredId), customerID(cId), totalAmount(total), orderDate(date) {
    if (restoredId > order_count)
        order_count = restoredId;
}

// Returns true for valid orders (orderID > 0), false for failed ones
// "null" order returned when checkout was skipped
bool Order::isValid() const { return orderID != -1; }

// Prints a receipt showing the order ID, customer and total
// const function
void Order::displayReceipt() const {
    if (!isValid()) {
        cout << "No valid order to display" << endl;
        return;
    }
    cout << endl << "========== RECEIPT ==========" << endl;
    cout << " .  Order ID  : " << left << setw(23) << orderID    << endl;
    cout << " .  Customer  : " << left << setw(23) << customerID  << endl;
    cout << " .  Date      : " << left << setw(23) << orderDate   << endl;
    cout << "=============================" << endl;
    cout << " .  Total     : $"
         << right << setw(8) << fixed << setprecision(2) << totalAmount << endl;
    cout << "=============================" << endl;
}

// getter functions
int Order::getOrderID() const { return orderID; }
const string& Order::getCustomerID() const { return customerID; }
double Order::getTotalAmount() const { return totalAmount; }
const string& Order::getOrderDate() const { return orderDate; }
int Order::getOrderCount() { return order_count; }

// =============================================================================
// Inventory  —  private helpers
// =============================================================================

// Find index of a product by ID
// Returns: index if found, -1 otherwise
int Inventory::indexOfId(const string &id) const {
    for (int i = 0; i < static_cast<int>(products.size()); i++)
        if (products[i]->getProductId() == id)
            return i;
    return -1;
}

// CSV helper: wraps string in quotes if it contains a comma
// Prevents breaking CSV structure
string Inventory::csvQuote(const string &s) {
    return (s.find(',') != string::npos) ? "\"" + s + "\"" : s;
}

// =============================================================================
// Inventory  —  public
// =============================================================================

Inventory::~Inventory() {
    for (auto *p : products)
        delete p;
}

// Add a new product (ensures unique ID)
bool Inventory::addProduct(Product *p) {
    if (!p) throw invalid_argument("Cannot add a null product.");
    if (indexOfId(p->getProductId()) != -1) {
        cout << "Product ID '" << p->getProductId() << "' already exists.\n";
        return false;
    }
    products.push_back(p);
    saveToFile();
    return true;
}

// Remove product by ID
bool Inventory::removeProduct(const string &id) {
    int index = indexOfId(id);
    if (index == -1) { cout << "Product with ID '" << id << "' not found.\n"; return false; }
    delete products[index];
    products.erase(products.begin() + index);
    saveToFile();
    return true;
}

// Searching about Prodcuts by differenet ways
// Way 1:
Product *Inventory::findById(const string &id) const {
    int index = indexOfId(id);
    return (index == -1) ? nullptr : products[index];
}

// Way 2:
vector<Product *> Inventory::findByName(const string &keyword) const {
    vector<Product *> results;
    string kw = keyword;
    transform(kw.begin(), kw.end(), kw.begin(), ::tolower);
    for (auto *p : products) {
        string n = p->getName();
        transform(n.begin(), n.end(), n.begin(), ::tolower);
        if (n.find(kw) != string::npos)
            results.push_back(p);
    }
    return results;
}

// Way 3:
vector<Product *> Inventory::findByBrand(const string &brand) const {
    vector<Product *> result;
    string br = brand;
    transform(br.begin(), br.end(), br.begin(), ::tolower);
    for (auto *p : products) {
        string b = p->getBrand();
        transform(b.begin(), b.end(), b.begin(), ::tolower);
        if (b == br)
            result.push_back(p);
    }
    return result;
}

// Way 4:
vector<Product *> Inventory::findByPriceRange(double minPrice, double maxPrice) const {
    if (minPrice < 0 || maxPrice < minPrice || maxPrice <= 0)
        throw invalid_argument("Invalid price range.");
    vector<Product *> result;
    for (auto *p : products)
        if (p->getCurrentPrice() >= minPrice && p->getCurrentPrice() <= maxPrice)
            result.push_back(p);
    return result;
}

// Stock operations
bool Inventory::restock(const string &id, int qty) {
    Product *p = findById(id);
    if (!p) {
        cout << "Restock failed: product '" << id << "' not found." << endl;
        return false;
    }
    p->restock(qty);
    saveToFile();
    return true;
}

bool Inventory::sell(const string &id, int qty) {
    Product *p = findById(id);
    if (!p) {
        cout << "Sale failed: product '" << id << "' not found." << endl;
        return false;
    }
    bool sold = p->sell(qty);
    if (sold)
        saveToFile();
    return sold;
}

// Pricing / status operations
bool Inventory::applyDiscount(const string &id, double amount, DiscountType type) {
    Product *p = findById(id);
    if (!p) {
        cout << "Discount failed: product '" << id << "' not found." << endl;
        return false;
    }
    p->setDiscountSale(amount, type);
    saveToFile();
    return true;
}

bool Inventory::discontinue(const string &id) {
    Product *p = findById(id);
    if (!p) {
        cout << "Discontinue failed: product '" << id << "' not found." << endl;
        return false;
    }
    p->discontinue();
    saveToFile();
    return true;
}

// Statistics
int Inventory::count() const { return static_cast<int>(products.size()); }
bool Inventory::isEmpty() const { return products.empty(); }
const vector<Product *> &Inventory::all() const { return products; }

int Inventory::countAvailable() const {
    int n = 0;
    for (auto *p : products) {
        if (p->isAvailable())
            ++n;
    }
    return n;
}

int Inventory::countOutOfStock() const {
    int n = 0;
    for (auto *p : products) {
        if (!p->isAvailable())
            ++n;
    }
    return n;
}

double Inventory::totalInventoryValue() const {
    double v = 0;
    for (auto *p : products)
        v += p->getCurrentPrice() * p->getStockQuantity();
    return v;
}

// Display functions
void Inventory::printAll() const {
    if (products.empty()) {
        cout << "No products in inventory.\n";
        return;
    }
    cout << "\n===== PRODUCT CATALOG (" << count() << " items) =====\n";
    for (auto *p : products)
        p->displayDetails();
}

void Inventory::printSearchResults(const string &keyword) const {
    auto res = findByName(keyword);
    if (res.empty()) {
        cout << "No products matching '" << keyword << "'.\n";
        return;
    }
    for (auto *p : res)
        p->displayDetails();
}

void Inventory::printOutOfStock() const {
    cout << "\n===== OUT-OF-STOCK / UNAVAILABLE =====\n";
    bool any = false;
    for (auto *p : products)
        if (!p->isAvailable()) {
            cout << "[" << p->getProductId() << "] "
                 << p->getName() << " - " << p->getStatusString() << "\n";
            any = true;
        }
    if (!any)
        cout << "All products are in stock!\n";
}

// Reporting
void Inventory::printInventorySummary() const {
    cout << "\n===== INVENTORY SUMMARY =====\n";
    cout << "Total Products  : " << count() << "\n";
    cout << "Available       : " << countAvailable() << "\n";
    cout << "Out of Stock    : " << countOutOfStock() << "\n";
    cout << "Inventory Value : $"
         << fixed << setprecision(2) << totalInventoryValue() << "\n";
    map<string, int> byType;
    for (auto *p : products)
        byType[p->getType()]++;
    cout << "By Type:\n";
    for (auto &[type, cnt] : byType)
        cout << "  " << type << ": " << cnt << "\n";
}

// File I/O - CSV format (41 columns, 0-indexed)
// Common    : 0:type 1:productId 2:sku 3:name 4:brand 5:model
//             6:mainPrice 7:currentPrice 8:discount 9:onSale
//             10:stockQuantity 11:description
// Smartphone: 12:ram 13:storage 14:batteryMah 15:inches 16:cameraMP
//             17:phoneOS 18:has5G 19:hasNFC 20:refreshRateHz 21:chipset
// Laptop    : 22:ramGB 23:storageGB 24:isSSD 25:displayInches 26:laptopOS
//             27:cpu 28:gpu 29:batteryWh 30:formFactor 31:weightKg
//             32:touchscreen 33:usbPorts 34:thunderbolt
// Accessory : 35:category 36:color 37:material 38:connectivity
//             39:isWireless 40:compatibleWith
// Unused columns for a type are written as "none".
// compatibleWith uses '|' as an internal separator.
void Inventory::saveToFile() const {
    // Opening File
    ofstream file(FILE_PATH);
    // Validation if not exists
    if (!file.is_open())
        throw runtime_error("Cannot open '" + FILE_PATH + "' for writing.");
    // Table Headers
    file << "type,productId,sku,name,brand,model,"
            "mainPrice,currentPrice,discount,onSale,stockQuantity,description,"
            "ram,storage,batteryMah,inches,cameraMP,phoneOS,has5G,hasNFC,refreshRateHz,chipset,"
            "ramGB,storageGB,isSSD,displayInches,laptopOS,cpu,gpu,batteryWh,"
            "formFactor,weightKg,touchscreen,usbPorts,thunderbolt,"
            "accessoryCategory,color,material,connectivity,isWireless,compatibleWith\n";
    // Rows as products
    for (auto *p : products) {
        const string &type = p->getType();
        // Checker if there is "," as usual string not csv format
        file << csvQuote(type)              << ","
             << csvQuote(p->getProductId()) << ","
             << csvQuote(p->getSku())       << ","
             << csvQuote(p->getName())      << ","
             << csvQuote(p->getBrand())     << ","
             << csvQuote(p->getModel())     << ","
             << p->getMainPrice()           << ","
             << p->getCurrentPrice()        << ","
             << p->getDiscount()            << ","
             << p->getOnSale()              << ","
             << p->getStockQuantity()       << ","
             << csvQuote(p->getDescription()) << ",";
        if (type == "smartphone") {
            // Dynamic Casting usage due to differnece in implemntation of each derived class
            auto *sp = dynamic_cast<SmartPhone *>(p);
            file << sp->getRam()        << ","
                 << sp->getStorage()    << ","
                 << sp->getBattery()    << ","
                 << sp->getDisplay()    << ","
                 << sp->getCamera()     << ","
                 << csvQuote(sp->getOS())      << ","
                 << sp->getHas5G()      << ","
                 << sp->getHasNFC()     << ","
                 << sp->getRefreshRate()<< ","
                 << csvQuote(sp->getChipset()) << ",";
            // setting all unrelated data as NONE
            file << "none,none,none,none,none,none,none,none,none,none,none,none,none,"
                    "none,none,none,none,none,none";
        } else if (type == "laptop") {
            // Dynamic Casting usage due to differnece in implemntation of each derived class
            auto *lp = dynamic_cast<Laptop *>(p);
            // setting all unrelated data as NONE
            file << "none,none,none,none,none,none,none,none,none,none,";
            file << lp->getRam()        << ","
                 << lp->getStorage()    << ","
                 << lp->getSSD()        << ","
                 << lp->getDisplay()    << ","
                 << csvQuote(lp->getOS())      << ","
                 << csvQuote(lp->getCPU())     << ","
                 << csvQuote(lp->getGPU())     << ","
                 << lp->getBatteryWh()  << ","
                 << static_cast<int>(lp->getFormFactor()) << ","
                 << lp->getWeightKg()   << ","
                 << lp->getTouchscreen()<< ","
                 << lp->getUsbPorts()   << ","
                 << lp->getThunderbolt()<< ",";
            // setting all unrelated data as NONE
            file << "none,none,none,none,none,none";
        } else {
            // Dynamic Casting usage due to differnece in implemntation of each derived class
            auto *ac = dynamic_cast<Accessory *>(p);
            // setting all unrelated data as NONE
            file << "none,none,none,none,none,none,none,none,none,none,"
                    "none,none,none,none,none,none,none,none,none,none,none,none,none,";
            file << static_cast<int>(ac->getCategory()) << ","
                 << csvQuote(ac->getColor())        << ","
                 << csvQuote(ac->getMaterial())     << ","
                 << csvQuote(ac->getConnectivity()) << ","
                 << ac->getIsWireless()             << ",";
            const auto &devs = ac->getCompatibleDevices();
            string compat;
            for (size_t i = 0; i < devs.size(); ++i) {
                if (i)
                    compat += "|";
                compat += devs[i];
            }
            file << csvQuote(compat.empty() ? "none" : compat);
        }
        file << "\n";
    }
    file.close();
    cout << "Inventory saved to " << FILE_PATH << " (" << count() << " products).\n";
}

void Inventory::loadFromFile() {
    ifstream file(FILE_PATH);
    if (!file.is_open()) {
        cout << "No existing warehouse file found at '" << FILE_PATH << "'. Starting fresh.\n";
        return;
    }
    // Lambda Function to convert CSV into suitable form (csv line to vector of strings)
    auto splitCSV = [](const string &row) -> vector<string> {
        vector<string> cols;
        string cur;
        bool inQuote = false;  // track if we are inside quotes
        for (char c : row) {
            if (c == '"')
                inQuote = !inQuote;  // toggle quote mode
            else if (c == ',' && !inQuote) {
                // comma outside quotes → new column
                cols.push_back(cur);
                cur.clear();
            }
            else
                cur += c;  // build current column
        }
        cols.push_back(cur);
        return cols;  // push last column
    };
    // Data already collected as strings so the following is lamdas to convert to the right type
    auto toInt  = [](const string &s){ return (s == "none" || s.empty()) ? 0 : stoi(s); };
    auto toDbl  = [](const string &s){ return (s == "none" || s.empty()) ? 0.0 : stod(s); };
    auto toBool = [](const string &s){ return s == "1"; };
    auto toStr  = [](const string &s){ return (s == "none") ? "" : s; };

    string line;  // line to be read from csv file
    getline(file, line);  // skip header
    vector<Product *> loaded;  // returned vector of products from existed file
    int lineNum = 1;
    while (getline(file, line)) {
        ++lineNum;
        if (line.empty())
            continue;

        vector<string> c = splitCSV(line);  // Apply the lamda function on every line
        if (c.size() < 41) {
            //Neglect any data with lenght less than what is expected
            cerr << "Warning: line " << lineNum << " has too few columns, skipping.\n";
            continue;
        }

        // Starting to build the vector product based on each type of products read from the file
        // Keeping in mind validation for the inputs
        const string &type = c[0];
        // using try and catch to negelect any wrong type even that its lenght is logically correct
        try {
            if (type == "smartphone") {
                auto *sp = new SmartPhone(
                    c[1], c[2], c[3], c[4], c[5],
                    toDbl(c[6]), toInt(c[10]),
                    toInt(c[12]), toInt(c[13]), toInt(c[14]),
                    toDbl(c[15]), toInt(c[16]), toStr(c[17]));
                sp->setDescription(toStr(c[11]));
                sp->setHas5G(toBool(c[18]));
                sp->setHasNFC(toBool(c[19]));
                if (toInt(c[20]) > 0)
                    sp->setRefreshRate(toInt(c[20]));
                sp->setChipset(toStr(c[21]));
                double disc = toDbl(c[8]);
                if (disc > 0)
                    sp->setDiscountSale(disc, DiscountType::FIXED_AMOUNT);
                loaded.push_back(sp);
            }
            else if (type == "laptop") {
                auto *lp = new Laptop(
                    c[1], c[2], c[3], c[4], c[5],
                    toDbl(c[6]), toInt(c[10]),
                    toInt(c[22]), toInt(c[23]), toBool(c[24]),
                    toDbl(c[25]), toStr(c[26]), toStr(c[27]), toStr(c[28]),
                    toInt(c[29]), static_cast<LaptopFormFactor>(toInt(c[30])),
                    toDbl(c[31]));
                lp->setDescription(toStr(c[11]));
                lp->setTouchscreen(toBool(c[32]));
                lp->setUsbPorts(toInt(c[33]));
                lp->setThunderbolt(toBool(c[34]));
                double disc = toDbl(c[8]);
                if (disc > 0)
                    lp->setDiscountSale(disc, DiscountType::FIXED_AMOUNT);
                loaded.push_back(lp);
            }
            else if (type == "accessory") {
                auto *ac = new Accessory(
                    c[1], c[2], c[3], c[4], c[5],
                    toDbl(c[6]), toInt(c[10]),
                    static_cast<AccessoryCategory>(toInt(c[35])),
                    toStr(c[36]), toStr(c[37]));
                ac->setDescription(toStr(c[11]));
                ac->setConnectivity(toStr(c[38]));
                ac->setWireless(toBool(c[39]));
                double disc = toDbl(c[8]);
                if (disc > 0)
                    ac->setDiscountSale(disc, DiscountType::FIXED_AMOUNT);
                string compat = toStr(c[40]);
                if (!compat.empty()) {
                    stringstream ss(compat);
                    string dev;
                    while (getline(ss, dev, '|'))
                        if (!dev.empty()) ac->addCompatibility(dev);
                }
                loaded.push_back(ac);
            }
            else {
                cerr << "Warning: unknown type '" << type << "' on line " << lineNum << ", skipping.\n";
            }
        }
        catch (const exception &e) {
            cerr << "Error on line " << lineNum << ": " << e.what() << " - skipping.\n";
        }
    }
    file.close();
    for (auto *p : products)
        delete p;
    products = move(loaded);
    cout << "Loaded " << count() << " products from " << FILE_PATH << ".\n";
}

// Operator overloading for making the add, remove easier
Inventory &Inventory::operator+=(Product *p) {
    addProduct(p);
    return *this;
}
Inventory &Inventory::operator-=(const string &id) {
    removeProduct(id);
    return *this;
}

// Summary output "Op Overloading"
// friend to access inevntory
ostream &operator<<(ostream &out, const Inventory &inv) {
    out << "Inventory: " << inv.count() << " product(s) | "
        << inv.countAvailable() << " available | "
        << "Value: $" << fixed << setprecision(2) << inv.totalInventoryValue();
    return out;
}

// =============================================================================
// ProductFactory
// =============================================================================

// KEY RESPONSIBILITIES:
// 1. Abstraction of Instantiation:
//    Provides static methods (createSmartPhone, createLaptop, createAccessory)
//    so the rest of the application doesn't need to know the specific
//    constructor details of each subclass
//
// 2. Pre-Creation Validation:
//    Enforces business logic guards before an object is even born. It ensures
//    that no product enters the system with invalid states (e.g., negative
//    prices, zero RAM, or impossible battery capacities)
//
// 3. SKU Generation:
//    Automatically handles the formatting of Stock Keeping Units (SKUs) based
//    on the product type (e.g., prefixing "SP-" for phones or "LP-" for laptops),
//    ensuring naming consistency across the database
//
// 4. Memory Management:
//    Handles the 'new' keyword allocation for heap-based objects, returning
//    pointers that can be managed by the Store's inventory vectors

// =============================================================================
// ProductFactory  —  private helper
// =============================================================================

// Input user validation
void ProductFactory::validateBase(double price, int stock, const string &context) {
    if (price <= 0)
        throw invalid_argument(context + ": price must be positive.");
    if (stock < 0)
        throw invalid_argument(context + ": stock cannot be negative.");
}

// =============================================================================
// ProductFactory  —  public
// =============================================================================

SmartPhone *ProductFactory::createSmartPhone(
    const string &id, const string &name, const string &brand,
    const string &model, double price, int stock,
    int ram, int storage, int battery, double display, int camera,
    const string &os, bool has5G, bool hasNFC,
    int refreshRate, const string &chip) {
    // Validation for base and special attributes
    validateBase(price, stock, "SmartPhone '" + name + "'");
    if (ram <= 0)
        throw invalid_argument("SmartPhone '" + name + "': RAM must be positive.");
    if (storage <= 0)
        throw invalid_argument("SmartPhone '" + name + "': storage must be positive.");
    if (battery <= 0)
        throw invalid_argument("SmartPhone '" + name + "': battery must be positive.");
    if (display <= 0)
        throw invalid_argument("SmartPhone '" + name + "': display size must be positive.");
    if (camera <= 0)
        throw invalid_argument("SmartPhone '" + name + "': camera MP must be positive.");
    if (refreshRate <= 0)
        throw invalid_argument("SmartPhone '" + name + "': refresh rate must be positive.");

    string sku = "SP-" + id;
    SmartPhone *sp = new SmartPhone(id, sku, name, brand, model,
                                    price, stock,
                                    ram, storage, battery,
                                    display, camera, os);
    sp->setHas5G(has5G);
    sp->setHasNFC(hasNFC);
    sp->setRefreshRate(refreshRate);
    sp->setChipset(chip);
    return sp;
}

Laptop *ProductFactory::createLaptop(
    const string &id, const string &name, const string &brand,
    const string &model, double price, int stock,
    int ram, int storage, bool ssd, double display,
    const string &os, const string &cpu, const string &gpu,
    int batteryWh, LaptopFormFactor form,
    double weightKg, bool touchScreen, int usbPorts, bool hasThunderbolt) {
    validateBase(price, stock, "Laptop '" + name + "'");
    if (ram <= 0)
        throw invalid_argument("Laptop '" + name + "': RAM must be positive.");
    if (storage <= 0)
        throw invalid_argument("Laptop '" + name + "': storage must be positive.");
    if (display <= 0)
        throw invalid_argument("Laptop '" + name + "': display size must be positive.");
    if (batteryWh <= 0)
        throw invalid_argument("Laptop '" + name + "': battery capacity must be positive.");
    if (weightKg < 0)
        throw invalid_argument("Laptop '" + name + "': weight cannot be negative.");
    if (usbPorts < 0)
        throw invalid_argument("Laptop '" + name + "': USB port count cannot be negative.");

    string sku = "LP-" + id;
    Laptop *lp = new Laptop(id, sku, name, brand, model,
                             price, stock,
                             ram, storage, ssd,
                             display, os, cpu, gpu,
                             batteryWh, form, weightKg);
    lp->setTouchscreen(touchScreen);
    lp->setUsbPorts(usbPorts);
    lp->setThunderbolt(hasThunderbolt);
    return lp;
}

Accessory *ProductFactory::createAccessory(
    const string &id, const string &name, const string &brand,
    const string &model, double price, int stock,
    AccessoryCategory cat, const string &color,
    const string &material, const string &connectivity,
    bool isWireless, const vector<string> &compatibleDevices) {
    validateBase(price, stock, "Accessory '" + name + "'");
    string sku = "AC-" + id;
    Accessory *ac = new Accessory(id, sku, name, brand, model,
                                   price, stock, cat, color, material);
    ac->setConnectivity(connectivity);
    ac->setWireless(isWireless);
    for (const string &device : compatibleDevices)
        ac->addCompatibility(device);
    return ac;
}

// =============================================================================
// User
// =============================================================================

// Hashing function (shared by all users)
string User::hashPassword(const string &password) {
    return to_string(hash<string>{}(password));
}

User::User(string n, string e, string p, string phNum, string Adds, bool isHashed) {
    id          = user_count++;
    name        = n;
    email       = e;
    phoneNumber = phNum;
    Address     = Adds;
    passwordHash = isHashed ? p : hashPassword(p);  //if not already hashed, hash it
}

// Restore constructor: sets id directly from file, password is always already hashed
User::User(int restoredId, string n, string e, string p, string phNum, string Adds) {
    id           = restoredId;
    name         = n;
    email        = e;
    phoneNumber  = phNum;
    Address      = Adds;
    passwordHash = p; // already hashed from file
}

// Default virtual implementations
void  User::browseProducts(Inventory&) { return; }
bool  User::addToCart(Product*, int) { return false; }
void  User::removeFromCart(string) { return; }
void  User::viewCart() const { return; }
Order User::checkout(Inventory&) { return Order(); }
void  User::addNewProduct(Inventory&) { return; }
void  User::deleteProduct(Inventory&, string) { return; }
void  User::restockProduct(Inventory&, string, int) { return; }
void  User::applyDiscountToProduct(Inventory&) { return; }
void  User::discontinueProduct(Inventory&, string) { return; }
void  User::viewInventoryReport(const Inventory&) const { return; }

int User::getID() { return id; }
string User::getName() { return name; }
string User::getEmail() { return email; }
string User::getPhNum() { return phoneNumber; }
string User::getAddress() { return Address; }
string User::getPasswordHash() { return passwordHash; }
bool User::checkPassword(const string &input) { return hashPassword(input) == passwordHash; }

// =============================================================================
// Customer
// =============================================================================

Customer::Customer(string n, string e, string p, string phNum, string Adds, bool isHashed)
    : User(n, e, p, phNum, Adds, isHashed) {}

Customer::Customer(int restoredId, string n, string e, string p, string phNum, string Adds)
    : User(restoredId, n, e, p, phNum, Adds) {}

char Customer::getRole() { return 'C'; }

// 1. Connection between users and inventory classes
void Customer::browseProducts(Inventory& inv) {
    cout << "Enter 1 to view all the products, ";
    cout << "Enter 2 to to search products by price range, ";
    cout << "Enter 3 to to search products by name: ";
    int x;
    cin >> x;
    cin.ignore();
    if (x == 1) {
        inv.printAll();
    }
    else if (x == 2) {
        double min, max;  // Prices are doubles in product class
        cout << "Enter minimum price: "; cin >> min;
        cout << "Enter maximum price: "; cin >> max;
        cin.ignore();

        // 1. Capture the vector returned by the Inventory
        vector<Product*> results = inv.Inventory::findByPriceRange(min, max);

        // 2. Check and Loop
        if (results.empty())
            cout << "No products found in that range." << endl;
        else {
            for (Product* p : results)
                p->displayDetails();
        }
    }
    else if (x == 3) {
        cout << "Enter the product's name you want: ";
        string name; getline(cin, name);
        vector<Product*> results = inv.findByName(name);
        if (results.empty())
            cout << "Sorry, no products found with that name." << endl;
        else {
            for (Product* p : results)
                p->displayDetails();
        }
    }
    else {
        cout << "wrong choice ... I'll view all the products to you." << endl;
        inv.printAll();
    }
}

// 2. Cart Connection (Addition)
bool Customer::addToCart(Product* p, int qty) {
    // check if p is valid
    if (p == nullptr) {
        cout << "Invalid item ... Failed to add this item to the cart." << endl;
        return false;
    }
    // check if quantity in stock is sufficient
    if (p->getStockQuantity() >= qty) {
        customerCart.addItem(p, qty);
        cout << "The item is successfully added to the cart." << endl;
        return true;
    }
    cout << "Unsufficient quantity ... Failed to add this item to the cart." << endl;
    return false;
}

// 3. Cart Connection (Removal)
void Customer::removeFromCart(string productId) {
    customerCart.removeItem(productId);
}

// 4. Cart Connection (Viewing)
void Customer::viewCart() const {
    customerCart.viewCart();
}

// 5. Inventory & Order Connection
Order Customer::checkout(Inventory& inv) {
    // 1. PRE-CHECK: Ensure the cart is not empty before proceeding
    if (customerCart.isEmpty()) {
        cout << "Checkout failed: Your cart is empty." << endl;
        return Order();  // Assuming a default constructor for a failed order
    }

    // 2. TOTAL CALCULATION: Sum up the costs of all items in the cart
    double grandTotal = 0.0;
    // We assume getItems() returns a collection of CartItems
    for (auto& entry : customerCart.getItems()) {
        // Calculate (Current Price * Quantity) for each item
        grandTotal += (entry.item->getCurrentPrice() * entry.quantity);
    }

    // 4. INVENTORY UPDATE: Deduct stock from the warehouse
    for (auto& entry : customerCart.getItems()) {
        // Calls the inventory's sell function which handles stock and status
        inv.sell(entry.item->getProductId(), entry.quantity);
    }

    // 5. RECORD CREATION: Create a new Order object (The Receipt)
    // Pass the customer's ID and the total price to the Order
    Order newOrder(to_string(this->id), grandTotal);

    // 6. CLEANUP: Clear the cart for future shopping
    customerCart.clear();

    // 7. FINAL FEEDBACK
    cout << "Order placed successfully! Total: $" << fixed << setprecision(2) << grandTotal << endl;

    // 8. RETURN
    return newOrder;
}

void Customer::viewOrderHistory(const vector<Order>& allOrders) const {
    string myId = to_string(id);
    bool found = false;
    cout << "\n===== YOUR ORDER HISTORY =====\n";
    for (const Order& ord : allOrders) {
        if (ord.getCustomerID() == myId) {
            ord.displayReceipt();
            found = true;
        }
    }
    if (!found) cout << "You have no orders yet.\n";
    cout << "==============================\n";
}

// =============================================================================
// Admin
// =============================================================================

Admin::Admin(string n, string e, string p, string phNum, string Adds, bool isHashed)
    : User(n, e, p, phNum, Adds, isHashed) {}

Admin::Admin(int restoredId, string n, string e, string p, string phNum, string Adds)
    : User(restoredId, n, e, p, phNum, Adds) {}

char Admin::getRole() { return 'A'; }

void Admin::addNewProduct(Inventory& inv) {
    int choice;
    cout << "Enter The type OF New Product:" << endl;
    cout << "choice Phone : enter 1" << endl;
    cout << "choice Laptop : enter 2" << endl;
    cout << "choice Accessory : enter 3" << endl;
    cin >> choice;
    cin.ignore();

    string id, name, brand, model;
    double price;
    int stock;

    cout << "Enter Product ID: ";
    getline(cin, id);
    cout << "Enter Name: ";
    getline(cin, name);
    cout << "Enter Brand: ";
    getline(cin, brand);
    cout << "Enter Model: ";
    getline(cin, model);
    cout << "Enter Price: ";
    cin >> price;
    cout << "Enter Stock Quantity: ";
    cin >> stock;

    Product* pro = nullptr;
    if (choice == 1) {
        int ram, storage, battery, camera, refreshRate;
        double display;
        string os, chipType;
        bool Has5G, hasnfs;

        cout << "Enter RAM: ";
        cin >> ram;
        cout << "Enter Storage: ";
        cin >> storage;
        cout << "Enter Battery (mAh): ";
        cin >> battery;
        cout << "Enter Display Size: ";
        cin >> display;
        cout << "Enter Camera MP: ";
        cin >> camera;
        cout << "Enter Refresh Rate: ";
        cin >> refreshRate;
        cout << "Enter 1 if Has 5g: ";
        cin >> Has5G;
        cout << "Enter 1 if Has NFC: ";
        cin >> hasnfs;
        cin.ignore();

        cout << "Enter Operating System: ";
        getline(cin, os);
        cout << "Enter Chip: ";
        getline(cin, chipType);
        pro = ProductFactory::createSmartPhone(id, name, brand, model, price, stock,
                                               ram, storage, battery, display, camera,
                                               os, Has5G, hasnfs, refreshRate, chipType);
    }
    else if (choice == 2) {
        int ram, storage, batteryWh, usbPorts;
        bool ssd, touchscreen, hasthunderbolt;
        double display, weight;
        string os, cpu, gpu;
        cout << "Enter RAM: ";
        cin >> ram;
        cout << "Enter Storage: ";
        cin >> storage;
        cout << "Enter Usb Ports: ";
        cin >> usbPorts;
        cout << "Enter 1 if has ssd: ";
        cin >> ssd;
        cout << "Enter 1 if has touchscreen: ";
        cin >> touchscreen;
        cout << "Enter 1 if has thunderbolt: ";
        cin >> hasthunderbolt;
        cout << "Enter Display Size: ";
        cin >> display;
        cin.ignore();

        cout << "Enter Operating System: ";
        getline(cin, os);
        cout << "Enter CPU: ";
        getline(cin, cpu);
        cout << "Enter GPU: ";
        getline(cin, gpu);
        cout << "Enter Battery Wh: ";
        cin >> batteryWh;
        cout << "Enter Weight: ";
        cin >> weight;

        pro = ProductFactory::createLaptop(id, name, brand, model, price, stock,
                                           ram, storage, ssd, display, os, cpu, gpu,
                                           batteryWh, LaptopFormFactor::GAMING,
                                           weight, touchscreen, usbPorts, hasthunderbolt);
    }
    else if (choice == 3) {
        int catChoice;
        bool IsWireless;
        string color, material, connectivity;

        cout << "Choose Category:\n";
        cout << "0. Case\n";
        cout << "1. Charger\n";
        cout << "2. Cable\n";
        cout << "3. Headphones\n";
        cout << "Enter choice: ";

        cin >> catChoice;
        cin.ignore();
        cout << "IS it Wireless?(1=yes,0=no)?"; cin >> IsWireless;
        cin.ignore();

        cout << "Enter Color: ";
        getline(cin, color);
        cout << "Enter Material: ";
        getline(cin, material);
        cout << "Enter Connecticity: ";
        getline(cin, connectivity);

        pro = ProductFactory::createAccessory(id, name, brand, model, price, stock,
                                              static_cast<AccessoryCategory>(catChoice),
                                              color, material, connectivity, IsWireless);
    }
    else {
        cout << "Invalid choice.\n";
        return;
    }
    // Add to inventory
    if (inv.addProduct(pro))
        cout << "Product added successfully!\n";
    else {
        delete pro;
        cout << "Failed to add product.\n";
    }
}

// 2. Deletion Connection
void Admin::deleteProduct(Inventory& inv, string productId) {
    if (inv.removeProduct(productId))
        cout << "Product deleted successfully.\n";
    else
        cout << "Failed to delete product.\n";
}

// 3. Stock Maintenance Connection
// Takes: Inventory reference, ID, and amount to add[cite: 3]
// Does: Finds product via inv.findById() and calls p->restock(qty)
// Returns: void
void Admin::restockProduct(Inventory& inv, string productId, int qty) {
    if (inv.restock(productId, qty))
        cout << "Product restocked successfully.\n";
    else
        cout << "Failed to restock product.\n";
}

void Admin::applyDiscountToProduct(Inventory& inv) {
    inv.printInventorySummary();
    string proId;
    cout << "Enter Product ID to discount: ";
    getline(cin, proId);
    if (!inv.findById(proId)) {
        cout << "Product not found.\n";
        return;
    }
    cout << "Discount type:\n  1) Percentage (%)\n  2) Fixed Amount ($)\nSelection: ";
    int dtype;
    cin >> dtype;
    cin.ignore();

    if (dtype != 1 && dtype != 2) {
        cout << "Invalid type.\n";
        return;
    }
    cout << "Enter discount value: ";
    double amount;
    cin >> amount;
    cin.ignore();

    DiscountType dt = (dtype == 1)?DiscountType::PERCENTAGE : DiscountType::FIXED_AMOUNT;
    try {
        if (inv.applyDiscount(proId, amount, dt))
            cout << "Discount applied successfully.\n";
        else
            cout << "Failed to apply discount.\n";
    } catch (const exception& e) {
        cout << "Error: " << e.what() << "\n";
    }
}

void Admin::discontinueProduct(Inventory& inv, string productId) {
    try {
        if (inv.discontinue(productId))
            cout << "Product marked as discontinued.\n";
        else
            cout << "Failed to discontinue product.\n";
    } catch (const exception& e) {
        cout << "Error: " << e.what() << "\n";
    }
}

// 4. System Reporting Connection
void Admin::viewInventoryReport(const Inventory& inv) const {
    inv.printInventorySummary();
    inv.printOutOfStock();
}

// =============================================================================
// userManager
// =============================================================================

userManager::userManager() {}

userManager* userManager::getUMinstance() {  // static method
    if (UMptr == nullptr)
        UMptr = new userManager();
    return UMptr;
}

// To detect if password is hashed or not ... to convert old users in the file
bool userManager::isHashed(const string& p) {
    // Check if all characters are digits and length is large
    if (p.length() < 10)
        return false;
    for (char c : p) {
        if (!isdigit(c))
            return false;
    }
    return true;
}

void userManager::ensureAdminExists() {
    for (auto* u : v)
        if (u->getRole() == 'A')
            return;  // at least one admin already exists
    cout << "\n[Setup] No admin account found. Please create the first admin.\n";
    createAdmin();
}

void userManager::createAdmin() {  // only user manager can create admins
    string name, email, password, phNum, ADDS;
    cout << "Please, enter admin's name: ";
    getline(cin, name);
    cout << "Please, enter admin's email: ";
    getline(cin, email);
    cout << "Please, enter admin's password: ";
    getline(cin, password);
    cout << "Please, enter admin's phone number: ";
    getline(cin, phNum);
    cout << "Please, enter admin's Address: ";
    getline(cin, ADDS);

    User* u = new Admin(name, email, password, phNum, ADDS);
    v.push_back(u);

    FILE* fptr = fopen("users.csv", "a");
    if (fptr == nullptr) {
        cout << "Error opening file!" << endl;
        return;
    }
    fprintf(fptr, "%d,%s,%s,%s,%s,%s,A\n",
            u->getID(), name.c_str(), email.c_str(),
            u->getPasswordHash().c_str(), phNum.c_str(), ADDS.c_str());
    fclose(fptr);

    cout << "Admin added successfully and saved to file.\n";
    cout << "The New Admin's ID is: " << u->getID() << endl;
}

void userManager::signUp() {
    cout << "Welcome to sign up!" << endl;
    string name, email, password, phNum, ADDS;
    cout << "Please, enter your name: ";
    getline(cin, name);  // To get a full line and accept spaces
    cout << "Please, enter your email: ";
    getline(cin, email);
    cout << "Please, enter your password: ";
    getline(cin, password);
    cout << "Please, enter your phone number: ";
    getline(cin, phNum);
    cout << "Please, enter user's Address: ";
    getline(cin, ADDS);

    // sign up for customers only
    User* u = new Customer(name, email, password, phNum, ADDS);
    v.push_back(u);

    FILE* fptr = fopen("users.csv", "a");

    if (fptr == nullptr) {
        cout << "Error opening file!" << endl;
        return;
    }

    fprintf(fptr, "%d,%s,%s,%s,%s,%s,C\n",
            u->getID(), name.c_str(), email.c_str(),
            u->getPasswordHash().c_str(),  // To save hased password instead of plain password
            phNum.c_str(), ADDS.c_str());
    fclose(fptr);

    cout << "You've signed up successfully" << endl;
    cout << "Your ID is " << u->getID() << endl;
}

User* userManager::login(Inventory& inv, vector<Order>& allOrders, FileManager& fm) {
    cout << "Welcome to login!" << endl;

    int ID;
    string P;
    cout << "Please, Enter your ID: ";
    cin >> ID;
    cin.ignore();

    for (int i = 0; i < (int)v.size(); i++)
    {
        if (v[i]->getID() == ID)
        {
            cout << "We've found your id." << endl;
            cout << "Please, Enter your password: ";
            getline(cin, P);

            if (v[i]->checkPassword(P))
            {
                cout << "You've logged in successfully" << endl;

                if (v[i]->getRole() == 'C')
                {
                    cout << "You're a customer." << endl;
                    Customer* cust = dynamic_cast<Customer*>(v[i]);
                    int customerChoice = 0;
                    while (customerChoice != 7)
                    {
                        cout << "\n--- Customer Menu ---\n"
                             << "1) Browse Products\n"
                             << "2) Add to Cart\n"
                             << "3) Remove from Cart\n"
                             << "4) View Cart\n"
                             << "5) Checkout\n"
                             << "6) My Order History\n"
                             << "7) Logout\n"
                             << "Selection: ";
                        if (!(cin >> customerChoice)) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "Invalid input. Please enter a number.\n";
                            continue;
                        }
                        cin.ignore();

                        if (customerChoice == 1) {
                            v[i]->browseProducts(inv);
                        }
                        else if (customerChoice == 2) {
                            string pId;
                            int qty;
                            cout << "Enter Product ID: "; getline(cin, pId);
                            cout << "Enter Quantity: "; cin >> qty; cin.ignore();
                            v[i]->addToCart(inv.findById(pId), qty);
                        }
                        else if (customerChoice == 3) {
                            string pId;
                            cout << "Enter Product ID to remove: ";
                            getline(cin, pId);
                            v[i]->removeFromCart(pId);
                        }
                        else if (customerChoice == 4) {
                            v[i]->viewCart();
                        }
                        else if (customerChoice == 5) {
                            Order receipt = v[i]->checkout(inv);
                            if (receipt.isValid()) {
                                receipt.displayReceipt();
                                allOrders.push_back(receipt);
                                inv.saveToFile();
                                fm.saveOrders();
                            }
                        }
                        else if (customerChoice == 6) {
                            if (cust)
                                cust->viewOrderHistory(allOrders);
                        }
                        else if (customerChoice == 7)
                            break;
                        else
                            cout << "Invalid choice. Please enter 1-7.\n";
                    }
                }
                else // Admin logic
                {
                    cout << "You're an admin." << endl;
                    int adminChoice = 0;
                    while (adminChoice != 8)
                    {
                        cout << "\n--- Admin Menu ---\n"
                             << "1) Add Product\n"
                             << "2) Delete Product\n"
                             << "3) Restock Product\n"
                             << "4) Apply Discount\n"
                             << "5) Discontinue Product\n"
                             << "6) Inventory Report\n"
                             << "7) Add Admin\n"
                             << "8) Logout\n"
                             << "Selection: ";
                        if (!(cin >> adminChoice)) { //if the user didn't enter a number
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "Invalid input. Please enter a number.\n";
                            continue;
                        }
                        cin.ignore();

                        if (adminChoice == 1) v[i]->addNewProduct(inv);
                        else if (adminChoice == 2) {
                            string proId; cout << "Product ID: "; getline(cin, proId);
                            v[i]->deleteProduct(inv, proId);
                        }
                        else if (adminChoice == 3) {
                            string proId; int qty;
                            cout << "Product ID: "; getline(cin, proId);
                            cout << "Quantity: "; cin >> qty; cin.ignore();
                            v[i]->restockProduct(inv, proId, qty);
                        }
                        else if (adminChoice == 4)
                            v[i]->applyDiscountToProduct(inv);
                        else if (adminChoice == 5) {
                            string proId; cout << "Product ID: "; getline(cin, proId);
                            v[i]->discontinueProduct(inv, proId);
                        }
                        else if (adminChoice == 6)
                            v[i]->viewInventoryReport(inv);
                        else if (adminChoice == 7)
                            createAdmin();
                        else if (adminChoice == 8)
                            break;
                        else
                            cout << "Invalid choice. Please enter 1-8.\n";
                    }
                }
                return v[i];
            }
            else {
                cout << "Incorrect password" << endl;
                return nullptr;
            }
        }
    }
    cout << "Login failed." << endl;
    return nullptr;
}

// Here, we will load the users from the file at the beginning
void userManager::loadFromFile() {
    FILE* fptr = fopen("users.csv", "r");

    if (fptr == nullptr) {
        cout << "Error opening file!" << endl;
        return;
    }

    // Clear current memory to avoid duplicates
    for (auto u : v)
        delete u;
    v.clear();

    char line[300];

    while (fgets(line, sizeof(line), fptr)) {
        stringstream ss(line);
        string idStr, name, email, password, phone, address, role;
        getline(ss,idStr,',');
        getline(ss,name,',');
        getline(ss,email,',');
        getline(ss,password,',');
        getline(ss,phone,',');
        getline(ss,address,',');
        getline(ss,role);

        // Strip \r from every field (handles Windows \r\n line endings)
        auto stripCR = [](string& s) { if (!s.empty() && s.back() == '\r') s.pop_back(); };
        stripCR(idStr); stripCR(name); stripCR(email);
        stripCR(password); stripCR(phone); stripCR(address); stripCR(role);

        // skip broken/empty lines
        if (idStr.empty() || role.empty())
            continue;

        int id;
        try {
            id = stoi(idStr);
        }
        catch (...) {
            continue;
        }

        User* u = nullptr;
        if (role == "A")
            u = new Admin(id, name, email, password, phone, address);
        else
            u = new Customer(id, name, email, password, phone, address);
        v.push_back(u);

        if (id > maxID)
            maxID = id;
    }
    fclose(fptr);
    //static id counter so new users don't duplicate idss
    User::user_count = maxID + 1;
}

// Right now: Old users get hashed in memory only, BUT file still has old plain passwords
// we need saveToFile() to rewrite on the file
void userManager::saveToFile() {
    FILE* fptr = fopen("users.csv", "w");

    if (fptr == nullptr) {
        cout << "Error opening file!" << endl;
        return;
    }
    for (int i = 0; i < (int)v.size(); i++) {
        User* u = v[i];

        fprintf(fptr, "%d,%s,%s,%s,%s,%s,%c\n",
                u->getID(), u->getName().c_str(), u->getEmail().c_str(),
                u->getPasswordHash().c_str(),  // hashed password
                u->getPhNum().c_str(),
                u->getAddress().c_str(),
                u->getRole());  // 'A' or 'C'
    }

    fclose(fptr);
    cout << "All users saved successfully (hashed passwords)." << endl;
}

void userManager::displayvector() {
    if (v.empty()) {
        cout << "No users to display.\n";
        return;
    }
    for (int i = 0; i < (int)v.size(); i++) {
        cout << "------------------------\n";
        cout << "ID: "      << v[i]->getID()    << endl;
        cout << "Name: "    << v[i]->getName()  << endl;
        cout << "Email: "   << v[i]->getEmail() << endl;
        cout << "Phone: "   << v[i]->getPhNum() << endl;
        cout << "Address: " << v[i]->getAddress()<< endl;
        cout << "Role: " << (v[i]->getRole() == 'A' ? "Admin" : "Customer") << "\n";
    }
}

userManager::~userManager() {  // To avoid memory leak
    for (int i = 0; i < (int)v.size(); i++)
        delete v[i];
}

// =============================================================================
// FileManager  —  private helper
// =============================================================================

// split a comma delimeter into fields
vector<string> FileManager::splitCSV(const string& line) {
    vector<string> fields;
    string cur;
    bool inQuote = false;
    for (char c : line) {
        if (c == '"')
            inQuote = !inQuote;
        else if (c == ',' && !inQuote) {
            fields.push_back(cur);
            cur.clear();
        }
        else cur += c;
    }
    fields.push_back(cur);
    return fields;
}

// =============================================================================
// FileManager  —  public
// =============================================================================

// FileManager DOES NOT OWN any of the instances
FileManager::FileManager(Inventory& inv, userManager& um, vector<Order>& ord)
    : inventory(inv), users(um), orders(ord) {}

// Delegates to Inventory::saveToFile() which writes the full csv
// though Warehouse.csv is called by Inventory on every iter we use it here for when we need a full save
void FileManager::saveProducts() {
    cout << "[FileManager] Saving products..." << endl;
    inventory.saveToFile();
}

// Delegates to Inventory::loadFromFile() which reads Warehouse.csv and reconstructs products
void FileManager::loadProducts() {
    cout << "[FileManager] Loading products..." << endl;
    inventory.loadFromFile();
}

// Delegates to userManager::saveToFile() which overwrites users.csv with using hashed passwords
// id,name,email,passwordHash,phone,address,role
void FileManager::saveUsers() {
    cout << "[FileManager] Saving users..." << endl;
    users.saveToFile();
}

// Delegates to userManager::loadFromFile() which reads users.csv and reconstructs Customer/Admin objects using isHashed()
// resets User::count to prevent collisions
void FileManager::loadUsers() {
    cout << "[FileManager] Loading users..." << endl;
    users.loadFromFile();
}

// Writes all valid orders to Orders.csv in comma delimited format
// orderID,customerID,totalAmount,orderDate
// Skips unsuccessful orders (orderID == -1)
void FileManager::saveOrders() {
    cout << "[FileManager] Saving orders..." << endl;
    ofstream file(ORDERS_FILE);
    if (!file.is_open()) {
        cout << "[FileManager] Error: cannot open " << ORDERS_FILE << endl;
        return;
    }

    // header
    file << "orderID,customerID,totalAmount,orderDate" << endl;

    int saved = 0;
    for (const Order& ord : orders) {
        if (!ord.isValid()) continue;
        file << ord.getOrderID()    << ","
             << ord.getCustomerID() << ","
             << fixed << setprecision(2) << ord.getTotalAmount() << ","
             << ord.getOrderDate()  << endl;
        saved++;
    }

    file.close();
    cout << "[FileManager] " << saved << " orders saved to " << ORDERS_FILE << endl;
}

// Read Orders.csv and rebuilds Order via the restoring constructor
// Order(int orderId, int customerId, double total, string date)
// Clears the current orders vector before loading to prevent duplicates
// Handles Order::count so new orders get unique ids
void FileManager::loadOrders() {
    cout << "[FileManager] Loading orders..." << endl;
    ifstream file(ORDERS_FILE);
    if (!file.is_open()) {
        cout << "[FileManager] No orders file found" << endl;
        return;
    }

    string line;
    getline(file, line);  // skip header
    orders.clear();
    int lineNum = 1, loaded = 0;

    while (getline(file, line)) {
        lineNum++;
        if (line.empty())
            continue;
        if (!line.empty() && line.back() == '\r')
            line.pop_back();
        vector<string> f = splitCSV(line);
        if (f.size() < 4) {
            cerr << "[FileManager] Warning: line " << lineNum
                 << " in Orders.csv has missing fields, skipping" << endl;
            continue;
        }
        try {
            // orderID, customerID, totalAmount, orderDate
            int ordId    = stoi(f[0]);
            string custId = f[1];
            double total = stod(f[2]);
            string date  = f[3];
            orders.push_back(Order(ordId, custId, total, date));
            loaded++;
        } catch (const exception& e) {
            cerr << "[FileManager] Error on line " << lineNum << ": " << e.what() << " , skipping" << endl;
        }
    }

    file.close();
    cout << "[FileManager] " << loaded << " orders loaded from " << ORDERS_FILE << endl;
}

void FileManager::saveAll() {
    saveProducts();
    saveUsers();
    saveOrders();
    cout << "[FileManager] All data is saved" << endl;
}

void FileManager::loadAll() {
    loadProducts();
    loadUsers();
    loadOrders();
    cout << "[FileManager] All data is loaded" << endl;
}
