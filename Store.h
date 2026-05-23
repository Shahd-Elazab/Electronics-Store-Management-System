#ifndef STORE_H
#define STORE_H

/*******************************************************************************
 * FILE: Store.h
 *
 * ROLE:
 * This is the single master header for the entire store system.
 * It contains all class declarations, enums, and inline method definitions
 * for the full product hierarchy, inventory, cart, order, user system,
 * product factory, and file manager.
 *
 * CONTENTS (in order):
 *   1. Product hierarchy  (Product, SmartPhone, Laptop, Accessory)
 *   2. Cart  (CartItem, Cart)
 *   3. Order
 *   4. Inventory
 *   5. ProductFactory
 *   6. User hierarchy (User, Customer, Admin, userManager)
 *   7. FileManager
 *******************************************************************************/

#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <limits>

using namespace std;

// =============================================================================
// Enums
// =============================================================================

enum class ProductStatus {
    AVAILABLE,
    OUT_OF_STOCK,
    DISCONTINUED,
    COMING_SOON
};

enum class DiscountType {
    PERCENTAGE,
    FIXED_AMOUNT
};

enum class LaptopFormFactor {
    ULTRABOOK,
    GAMING,
    WORKSTATION,
    CONVERTIBLE,
    CHROMEBOOK
};

enum class AccessoryCategory {
    CASE_COVER,
    CHARGER,
    CABLE,
    HEADPHONES,
    SCREEN_PROTECTOR,
    KEYBOARD,
    MOUSE
};

// =============================================================================
// Product (abstract base)
// =============================================================================

class Product {
private:
    string productId;
    string sku;
    string name;
    string brand;
    string model;
    string description;
    double mainPrice;
    double currentPrice;
    double discount;
    bool onSale;
    ProductStatus status;
    int stockQuantity;

    void updateStatus();
    string statusToString() const;

protected:
    void displayBase(ostream &out) const;
    Product();

public:
    Product(const string &id, const string &sku,
            const string &name, const string &brand,
            const string &model, double price, int stock);
    virtual ~Product();

    virtual void displayDetails() const = 0;
    virtual string getType() const = 0;
    virtual void print(ostream &out) const;

    // Getters
    const string &getProductId() const;
    const string &getSku() const;
    const string &getName() const;
    const string &getBrand() const;
    const string &getModel() const;
    const string &getDescription() const;
    double getMainPrice() const;
    double getCurrentPrice() const;
    double getDiscount() const;
    bool getOnSale() const;
    int getStockQuantity() const;
    ProductStatus getStatus() const;
    string getStatusString() const;

    // Setters / operations
    void setDescription(const string &d);
    void setDiscountSale(double amount, DiscountType type);
    void resetDiscount();
    void markComingSoon();
    void discontinue();
    double getFinalPrice(double taxRate = 0.0) const;
    bool sell(int quantity);
    void restock(int quantity);
    bool isAvailable() const;

    bool operator==(const Product &other) const;
    bool operator<(const Product &other) const;
    friend ostream &operator<<(ostream &out, const Product &p);
};

// =============================================================================
// SmartPhone
// =============================================================================

class SmartPhone : public Product {
private:
    int ram;
    int storage;
    int batteryMah;
    double inches;
    int cameraMP;
    string phoneOS;
    bool has5G;
    bool hasNFC;
    int refreshRateHz;
    string chipset;

public:
    SmartPhone(const string &id, const string &sku,
               const string &name, const string &brand,
               const string &model, double price, int stock,
               int ram, int storage, int battery,
               double display, int camera, const string &os);

    // Setters
    void setHas5G(bool v);
    void setHasNFC(bool v);
    void setChipset(const string &c);
    void setOS(const string &os);
    void setRefreshRate(int hz);
    void setRam(int r);
    void setStorage(int s);
    void setBattery(int mah);
    void setDisplay(double d);
    void setCamera(int mp);

    // Getters
    int getRam() const;
    int getStorage() const;
    int getBattery() const;
    double getDisplay() const;
    int getCamera() const;
    const string &getOS() const;
    bool getHas5G() const;
    bool getHasNFC() const;
    int getRefreshRate() const;
    const string &getChipset() const;

    bool isFlagship() const;
    bool isGamingPhone() const;
    bool isMidRange() const;
    double estimatedBatteryDays(int mAhPerDay = 1000) const;

    string getType() const override;
    void displayDetails() const override;
    void print(ostream &out) const override;
};

// =============================================================================
// Laptop
// =============================================================================

class Laptop : public Product {
private:
    int ramGB;
    int storageGB;
    bool isSSD;
    double displayInches;
    string operatingSystem;
    string cpu;
    string gpu;
    int batteryWh;
    LaptopFormFactor formFactor;
    bool hasTouchscreen;
    int usbPorts;
    bool hasThunderbolt;
    double weightKg;

    string formFactorToString() const;

public:
    Laptop(const string &id, const string &sku,
           const string &name, const string &brand,
           const string &model, double price, int stock,
           int ram, int storage, bool ssd,
           double display, const string &os,
           const string &cpu, const string &gpu,
           int batteryWh, LaptopFormFactor form,
           double weightKg = 0.0);

    // Setters
    void setTouchscreen(bool v);
    void setThunderbolt(bool v);
    void setFormFactor(LaptopFormFactor f);
    void setSSD(bool v);
    void setOS(const string &os);
    void setCPU(const string &c);
    void setGPU(const string &g);
    void setUsbPorts(int n);
    void setRam(int r);
    void setStorage(int s);
    void setDisplay(double d);
    void setBatteryWh(int wh);
    void setWeight(double kg);

    // Getters
    int getRam() const;
    int getStorage() const;
    bool getSSD() const;
    double getDisplay() const;
    const string &getOS() const;
    const string &getCPU() const;
    const string &getGPU() const;
    int getBatteryWh() const;
    LaptopFormFactor getFormFactor() const;
    bool getTouchscreen() const;
    int getUsbPorts() const;
    bool getThunderbolt() const;
    double getWeightKg() const;

    bool hasDiscreteGPU() const;
    bool isWorkhorseReady() const;
    bool isUltrathin() const;
    double estimatedBatteryHours(int avgWatts = 10) const;

    string getType() const override;
    void displayDetails() const override;
    void print(ostream &out) const override;
};

// =============================================================================
// Accessory
// =============================================================================

class Accessory : public Product {
private:
    AccessoryCategory category;
    vector<string> compatibleWith;
    string color;
    string material;
    string connectivity;
    bool isWireless;

    string categoryToString() const;

public:
    Accessory(const string &id, const string &sku,
              const string &name, const string &brand,
              const string &model, double price, int stock,
              AccessoryCategory cat,
              const string &color = "", const string &material = "");

    void addCompatibility(const string &device);
    void removeCompatibility(const string &device);
    void clearCompatibility();
    bool isCompatibleWith(const string &device) const;
    const vector<string> &getCompatibleDevices() const;
    int getCompatibilityCount() const;

    // Setters
    void setConnectivity(const string &c);
    void setWireless(bool v);
    void setColor(const string &c);
    void setMaterial(const string &m);
    void setCategory(AccessoryCategory c);

    // Getters
    AccessoryCategory getCategory() const;
    const string &getColor() const;
    const string &getMaterial() const;
    const string &getConnectivity() const;
    bool getIsWireless() const;

    string getType() const override;
    void displayDetails() const override;
    void print(ostream &out) const override;
};

// =============================================================================
// CartItem & Cart
// =============================================================================

struct CartItem {
    Product* item;
    int quantity;
};

class Cart {
private:
    vector<CartItem> items;
    double totalPrice;

public:
    Cart();
    void addItem(Product* p, int qnty);
    void removeItem(const string& productId);
    void viewCart() const;
    bool isEmpty() const;
    const vector<CartItem>& getItems() const;
    void clear();
    double getTotal() const;
};

// =============================================================================
// Order
// =============================================================================

class Order {
private:
    int orderID;
    string customerID;
    double totalAmount;
    string orderDate;
    static int order_count;

    static string currentDate();

public:
    Order(const string& customerId, double grandTotal);
    Order();
    Order(int restoredId, const string& cId, double total, const string& date);

    bool isValid() const;
    void displayReceipt() const;

    int getOrderID() const;
    const string& getCustomerID() const;
    double getTotalAmount() const;
    const string& getOrderDate() const;
    static int getOrderCount();
};

// =============================================================================
// Inventory
// =============================================================================

class Inventory {
private:
    vector<Product*> products;
    const string FILE_PATH = "Warehouse.csv";

    int indexOfId(const string &id) const;
    static string csvQuote(const string &s);

public:
    Inventory() = default;
    Inventory(const Inventory &) = delete;
    Inventory &operator=(const Inventory &) = delete;
    ~Inventory();

    bool addProduct(Product *p);
    bool removeProduct(const string &id);

    Product *findById(const string &id) const;
    vector<Product *> findByName(const string &keyword) const;
    vector<Product *> findByBrand(const string &brand) const;
    vector<Product *> findByPriceRange(double minPrice, double maxPrice) const;

    bool restock(const string &id, int qty);
    bool sell(const string &id, int qty);
    bool applyDiscount(const string &id, double amount, DiscountType type);
    bool discontinue(const string &id);

    int count() const;
    bool isEmpty() const;
    const vector<Product *> &all() const;
    int countAvailable() const;
    int countOutOfStock() const;
    double totalInventoryValue() const;

    void printAll() const;
    void printSearchResults(const string &keyword) const;
    void printOutOfStock() const;
    void printInventorySummary() const;

    void saveToFile() const;
    void loadFromFile();

    Inventory &operator+=(Product *p);
    Inventory &operator-=(const string &id);
    friend ostream &operator<<(ostream &out, const Inventory &inv);
};

// =============================================================================
// ProductFactory
// =============================================================================

class ProductFactory {
    static void validateBase(double price, int stock, const string &context);

public:
    static SmartPhone *createSmartPhone(
        const string &id, const string &name, const string &brand,
        const string &model, double price, int stock,
        int ram, int storage, int battery, double display, int camera,
        const string &os, bool has5G = false, bool hasNFC = false,
        int refreshRate = 60, const string &chip = "Unknown");

    static Laptop *createLaptop(
        const string &id, const string &name, const string &brand,
        const string &model, double price, int stock,
        int ram, int storage, bool ssd, double display,
        const string &os, const string &cpu, const string &gpu,
        int batteryWh, LaptopFormFactor form,
        double weightKg = 0.0, bool touchScreen = false,
        int usbPorts = 0, bool hasThunderbolt = false);

    static Accessory *createAccessory(
        const string &id, const string &name, const string &brand,
        const string &model, double price, int stock,
        AccessoryCategory cat, const string &color = "",
        const string &material = "", const string &connectivity = "",
        bool isWireless = false,
        const vector<string> &compatibleDevices = {});
};

// =============================================================================
// Forward declarations needed by User
// =============================================================================

extern int maxID;

// =============================================================================
// User (abstract base)
// =============================================================================

class User {
protected:
    int id;
    string name;
    string email;
    string passwordHash;
    string phoneNumber;
    string Address;

    static string hashPassword(const string &password);

public:
    static int user_count;

    User(string n, string e, string p, string phNum, string Adds, bool isHashed = false);
    User(int restoredId, string n, string e, string p, string phNum, string Adds); // restore from file
    virtual ~User() {}

    virtual char getRole() = 0;

    // Customer virtual functions
    virtual void browseProducts(Inventory& inv);
    virtual bool addToCart(Product* p, int qty);
    virtual void removeFromCart(string productId);
    virtual void viewCart() const;
    virtual Order checkout(Inventory& inv);

    // Admin virtual functions
    virtual void addNewProduct(Inventory& inv);
    virtual void deleteProduct(Inventory& inv, string productId);
    virtual void restockProduct(Inventory& inv, string productId, int qty);
    virtual void applyDiscountToProduct(Inventory& inv);
    virtual void discontinueProduct(Inventory& inv, string productId);
    virtual void viewInventoryReport(const Inventory& inv) const;

    // Getters
    int getID();
    string getName();
    string getEmail();
    string getPhNum();
    string getAddress();
    string getPasswordHash();
    bool checkPassword(const string &input);
};

// =============================================================================
// Customer
// =============================================================================

class Customer : public User {
private:
    Cart customerCart;

public:
    Customer(string n, string e, string p, string phNum, string Adds, bool isHashed = false);
    Customer(int restoredId, string n, string e, string p, string phNum, string Adds); // restore from file
    char getRole() override;
    const Cart& getCart() const { return customerCart; }  // ← ADD THIS LINE

    void browseProducts(Inventory& inv) override;
    bool addToCart(Product* p, int qty) override;
    void removeFromCart(string productId) override;
    void viewCart() const override;
    Order checkout(Inventory& inv) override;
    void viewOrderHistory(const vector<Order>& allOrders) const;
};

// =============================================================================
// Admin
// =============================================================================

class Admin : public User {
public:
    Admin(string n, string e, string p, string phNum, string Adds, bool isHashed = false);
    Admin(int restoredId, string n, string e, string p, string phNum, string Adds); // restore from file
    char getRole() override;

    void addNewProduct(Inventory& inv) override;
    void deleteProduct(Inventory& inv, string productId) override;
    void restockProduct(Inventory& inv, string productId, int qty) override;
    void applyDiscountToProduct(Inventory& inv) override;
    void discontinueProduct(Inventory& inv, string productId) override;
    void viewInventoryReport(const Inventory& inv) const override;
};

// =============================================================================
// Forward declaration so userManager::login() can accept FileManager&
// without a circular include (FileManager is fully declared below).
// =============================================================================

class FileManager;

// =============================================================================
// userManager (Singleton)
// =============================================================================

class userManager {
private:
    vector<User*> v;
    userManager();
    static userManager* UMptr;

public:
    // --- ADD THIS LINE ---
    const vector<User*>& getUsers() const { return v; }
    userManager(const userManager&) = delete;
    userManager& operator=(const userManager&) = delete;

    static userManager* getUMinstance();

    bool isHashed(const string& p);
    void ensureAdminExists();
    void createAdmin();
    void signUp();
    User* login(Inventory& inv, vector<Order>& allOrders, FileManager& fm);
    void loadFromFile();
    void saveToFile();
    void displayvector();

    ~userManager();
};

// =============================================================================
// FileManager
// =============================================================================

class FileManager {
private:
    Inventory&    inventory;
    userManager&  users;
    vector<Order>& orders;

    const string ORDERS_FILE = "Orders.csv";

    static vector<string> splitCSV(const string& line);

public:
    FileManager(Inventory& inv, userManager& um, vector<Order>& ord);
    FileManager(const FileManager&) = delete;
    FileManager& operator=(const FileManager&) = delete;

    void saveProducts();
    void loadProducts();
    void saveUsers();
    void loadUsers();
    void saveOrders();
    void loadOrders();
    void saveAll();
    void loadAll();
};

#endif // STORE_H