# YTX Item Management

Hello everyone! Today, I'll introduce how **YTX** manages items.

In traditional manufacturing, knowing material location, quality, and availability directly affects inventory efficiency. **YTX** solves the problem of hard-to-find materials and greatly improves warehouse efficiency.

Current software version: **0.2.5**

---

## Main Interface

1. **Login**
   - Enter **email**, **password**, and **workspace** (equivalent to company name).
   - Optionally choose to **save password locally**.

2. **Menus**
   - **Menu Bar**: File, Edit, Account
   - **Toolbar**: Insert Node, Append Node, Edit Node Name, Reset Node Color, Create Node Group, Append Record, Delete, Select All, Invert Selection, Clear Selection, Bill, Settlement
   - **Section Bar**: Finance, Item, Stakeholder, Task

3. **Item Section**
   - Also called **Material Section**、**Product Section** or **Warehouse Section**, unified as **Item**
   - Initially empty; structure is built via **nodes**

---

## Core Properties

Each node has **three key attributes**:

- **Rule**: DICD or DDCI, determines how leaf node values are calculated
- **Type**:
  - **B (Branch)**: Group node used for aggregating leaf data
  - **L (Leaf)**: Final node for data entry
- **Unit**:
  - **Internal**: Own number
  - **External**: Mapping number
  - **Empty**: Position
  - Internal number and position are used most; external numbers map relationships

---

## Simulation

### Nodes

1. **Add a product branch node**
   - Shortcut: `Alt + P`
   - Name: "Apple", Type: "B", Unit: "Internal"

2. **Add three leaf nodes under Apple**
   - Select Apple → `Alt + P`
   - Names: "Phone", "Laptop", "Desktop", Type: "L", Unit: "Internal"

3. **Add a position branch node under Apple**
   - Select Apple → `Alt + N` (same layer insert)
   - Name: "Location", Type: "B", Unit: Empty

4. **Add three leaf nodes under Location**
   - Select Location → `Alt + P`
   - Names: "Box 1", "Box 2", "Box 3", Type: "L", Unit: Empty

> You can now expand/collapse branches or drag nodes.

**Notes:**

- `Alt + P` Append a child node to the currently selected branch node; if no node is selected, append a child node to root, which becomes the top-level node visible in the interface.
- `Alt + N` Insert a new node at the same level as the currently selected node; if no node is selected, no action is performed.
- Type and unit **cannot be modified**; incorrect nodes must be deleted and re-added.
- Branch nodes only aggregate leaf nodes with the **same unit**.

---

### Records

1. **Add a record**
   - Double-click `Apple - Phone` → Record page
   - Toolbar highlights available operations

2. **Key attributes**
   - **D**: Add related documents (path only, no actual storage)
   - **C**: Used during inventory check
   - **Association**: Link to **Location** or other **Products**
     - Position → where it is stored
     - Product → next production step
   - **Debit / Credit / Balance**:
     - Debit and credit are used to record quantities.
     - Debit and credit must always come in pairs. The debit/credit data of the current node must match the credit/debit data of the associated node.
     - Balance calculation depends on rule:
       - DICD: Debit − Credit
       - DDCI: Credit − Debit

3. **Example**
   - Insert record: `Ctrl + N`
   - Associate with `Location - Box 3`
   - Debit: 50, Cost: 100
   - Jump to associated node: `Ctrl + J`
   - Return to item section to see entered data

4. **Modify rules**
   - Change all location nodes to **DDCI**
   - Now data entry is complete:
     - Check contents in `Box 3`
     - Check location of `Phone`

5. **Search**
   - `Ctrl + F` to find nodes by name or record description

---

That concludes the **Item Section**.
Wish you a productive day!
