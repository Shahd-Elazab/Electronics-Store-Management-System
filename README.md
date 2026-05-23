# Electronics Store Management System

<div align="left">

![C++](https://img.shields.io/badge/C++-20-blue?style=for-the-badge&logo=cplusplus)
![OOP](https://img.shields.io/badge/OOP-Architecture-red?style=for-the-badge)
![Slint](https://img.shields.io/badge/GUI-Slint-green?style=for-the-badge)
![STL](https://img.shields.io/badge/STL-DataStructures-orange?style=for-the-badge)
![CSV](https://img.shields.io/badge/Database-CSV-yellow?style=for-the-badge)

A powerful and modular Electronics Store Management System developed using C++ and Object-Oriented Programming principles.  
The project simulates a real-world electronics store environment, allowing efficient management of products, inventory, carts, customers, and orders through a structured and scalable architecture.

---

# ✨ Key Features

## 🛍️ Customer Features
- User registration & login
- Secure password authentication
- Product browsing & filtering
- Shopping cart management
- Checkout & order placement
- Order history review
- Dynamic receipt generation

---

## 🛠️ Admin Features
- Add new products
- Delete products
- Restock inventory
- Apply discounts
- Mark products as:
  - Available
  - Out of Stock
  - Coming Soon
  - Discontinued
- Monitor inventory summaries
- Monitor users & orders

---

## 💾 Persistence Features
- Automatic CSV synchronization
- Persistent storage across sessions
- Dynamic loading/saving of:
  - Users
  - Products
  - Orders

---

## 🔒 Security Features
- Password hashing
- Controlled authentication
- Role-based authorization
- Protected admin access
## Technologies & Concepts

---
# 🏗️ System Architecture

The system provides:

- 👤 Multi-role user management
- 📦 Smart inventory & warehouse tracking
- 🛒 Dynamic shopping cart workflows
- 📄 Persistent order management
- 🔐 Secure password hashing & authentication
- 💾 CSV-based database persistence
- 🖥️ GUI integration using Slint
- ⚙️ Runtime polymorphic product handling
- 📊 STL-based optimized data structures
- 🏗️ Design-pattern-driven architecture

### Languages & Tools
- C++
- Slint GUI Toolkit
- Console-Based Programming

### OOP Concepts
- Classes & Objects
- Inheritance
- Polymorphism
- Encapsulation
- Abstraction

### Data Structures
- Maps
- Vectors
- Linked Structures

### Additional Concepts
- File Handling
- Exception Handling
- Modular Programming

---

# The project follows a **Layered Enterprise Architecture**.

```text
┌──────────────────────────────┐
│     Presentation Layer       │
│ ──────────────────────────── │
│  • Slint GUI                 │
│  • Console Interface         │
└──────────────────────────────┘

┌──────────────────────────────┐
│      Business Logic Layer    │
│ ──────────────────────────── │
│  • UserManager               │
│  • Inventory                 │
│  • Cart                      │
│  • Orders                    │
│  • ProductFactory            │
└──────────────────────────────┘

┌──────────────────────────────┐
│       Persistence Layer      │
│ ──────────────────────────── │
│  • FileManager               │
│  • users.csv                 │
│  • warehouse.csv             │
│  • orders.csv                │
└──────────────────────────────┘
```

# Project Structure
```bash

├── gui/
│   └── appwindow.slint      # Slint UI design layout specification
├── CMakeLists.txt           # Build configuration linking both executable targets
├── Store.h                  # Master header: polymorphic schemas & class declarations
├── Store.cpp                # Core implementation: Inventory, Cart, Order, & userManager
├── GuiController.h          # GUI bridge controller header
├── GuiController.cpp        # GUI event routing & state synchronization
├── main.cpp                 # Execution entry point for the CONSOLE application
├── main_gui.cpp             # Execution entry point for the SLINT GUI application
├── users.csv                # Persistent security database (encrypted password hashes)
├── Warehouse.csv            # Positional subschema tracking database for products
└── Orders.csv               # Transaction history & immutable receipt database
