# YTX Inventory Tutorial

Hello everyone! Today we'll introduce how **YTX** manages inventory.

The current software version is **0.3.2**, and it can be downloaded from [GitHub](https://github.com/ytxerp/ytx-client/releases/tag/0.3.2).

---

## Main Interface

First, let's take a look at the main interface.

### 1. Login

Login requires three pieces of information:
- **Email**: Self-registered by users
- **Password**: Set during registration
- **Workspace**: Created by administrators

**Workflow:**
1. Users register with their email and password
2. Users apply to join a workspace
3. Administrators set appropriate permissions and grant access

**Tip:** You can choose to save your password locally for convenient future logins.

### 2. Menu

The interface is organized into three main areas:

- **Menu Bar** (Top): Contains three groups
  - File
  - Edit
  - Account

- **Toolbar** (Below menu): Commonly used functions include
  - Insert Node, Append Node, Edit Node Name, Reset Node Color
  - Create Node Group
  - Append Record, Delete
  - Select All, Invert Selection, Clear Selection
  - Bill, Settlement

- **Section Bar** (Left): All available sections
  - Finance
  - Task
  - Inventory
  - Partner
  - Sale and Purchase

### 3. Inventory

Inventory may have different names in various contexts (item, product, warehouse, etc.), but they all refer to the same concept.

By default, the interface is empty and requires creating **nodes** to gradually build your inventory structure.

---

## Node Attributes

Understanding the **three core attributes** of nodes is essential:

### Rule
Determines how the node balance is calculated:
- **DICD**: Balance = Debit − Credit
- **DDCI**: Balance = Credit − Debit

### Type
Defines the node's behavior:
- **B (Branch Node)**: Aggregates data from all leaf nodes under it
- **L (Leaf Node)**: Where data is actually entered

### Unit
Specifies what the node represents:
- **INT (Internal SKU)**: Your company's product code
- **EXT (External SKU)**: Customer's purchase code or supplier's sales code. When creating orders, external SKUs will be automatically imported based on internal SKUs
- **POS (Position)**: Indicates where the product is physically stored

**Usage Note:** In practice, internal SKU and position are most commonly used. External SKU is used for mapping relationships and is configured in the **Collaboration** section.

---

## Demonstration

Let's see how the Inventory section works through a practical example.

### Creating the Node Structure

**Step 1: Add a Product Branch Node**
- Shortcut: `Alt + N`
- Name: `Apple`
- Type: `B` (Branch)
- Unit: `INT` (Internal SKU)

**Step 2: Append Product Child Nodes**
- Select `Apple` node
- Press `Alt + P` to append children
- Create three leaf nodes:
  - Name: `Phone`, Type: `L`, Unit: `INT`
  - Name: `Computer`, Type: `L`, Unit: `INT`
  - Name: `Tablet`, Type: `L`, Unit: `INT`

**Step 3: Add a Position Branch Node**
- Select `Apple` node
- Press `Alt + N` to insert at the same level
- Name: `Position`
- Type: `B` (Branch)
- Unit: `POS` (Position)

**Step 4: Append Position Child Nodes**
- Select `Position` node
- Press `Alt + P` to append children
- Create three leaf nodes:
  - Name: `Box 1`, Type: `L`, Unit: `POS`
  - Name: `Box 2`, Type: `L`, Unit: `POS`
  - Name: `Box 3`, Type: `L`, Unit: `POS`

**Step 5: Organizing Nodes**
- Incorrect nodes can be deleted directly
- Expand or collapse nodes as needed
- Drag nodes to adjust their positions

### Important Notes

**Node Operations:**
- **Append (`Alt + P`)**: Appends a child node under the selected branch node. If no node is selected, the operation will be invalid.
- **Insert (`Alt + N`)**: Inserts a new node at the same level as the selected node. If no node is selected, it will be appended to the root level (creating a top-level node).

**Critical Restrictions:**
- Once a node's **Type** and **Unit** are set, they **cannot be modified**. If entered incorrectly, you must delete and recreate the node.
- Branch nodes will only aggregate data from leaf nodes with the same **Unit**.
- The unit of **Position** nodes must be accurate to ensure correct data collection when creating orders in the Order section.

---

### Working with Records

Now let's learn how to enter and manage data.

#### Adding Records

1. **Enter Operation Page**
   - Double-click `Apple - Phone` to enter the operation page
   - The toolbar will automatically highlight available functions

2. **Record Attributes**

   **D (Documents)**
   - Used to link related documents
   - Only the file path is saved, not the actual file

   **S (Status)**
   - Represents record status
   - Commonly used for verification workflows

   **Association**
   - Can be associated with **Position** or **Other Products**
   - **Position Association**: Indicates storage location
   - **Product Association**: Represents the product moving to the next stage

   **Debit and Credit**
   - Used to record transactions and always appear in pairs
   - When the current node has data in **Debit**, the associated node must have the same data in **Credit**
   - When the current node has data in **Credit**, the associated node must have the same data in **Debit**

   **Balance**
   - If node rule is `DICD`: Balance = Debit − Credit
   - If node rule is `DDCI`: Balance = Credit − Debit

#### Example Workflow

1. **Create a New Record**
   - Press `Ctrl + N`
   - Set association to `Position - Box 3`
   - Enter 50 for debit
   - Enter 7749 for cost

2. **Navigate to Associated Node**
   - Use `Ctrl + J` shortcut to quickly jump to the associated node's page
   - Double-click the title to return to the Inventory section
   - Verify that the data has been synchronized to the node

#### Viewing and Searching Data

**Two-Way Data Query:**
- In `Box 3`: View what specific items are stored inside
- In `Phone`: View which locations it's stored in

**Quick Search:**
- Press `Ctrl + F` to search by **Node Name** or **Entry Description**

---

## Summary

That's the introduction to **YTX Inventory**! After understanding the logic of nodes and records, you can easily design different structures to adapt to your actual needs, such as:
- Managing household items
- Tracking company inventory
- Organizing warehouse locations

**Key Takeaways:**
- Nodes create the organizational structure (Branch for categories, Leaf for actual items)
- Records track the movement and status of inventory
- The double-entry system (Debit/Credit) ensures data accuracy across associations

We hope YTX can be helpful in managing your inventory efficiently. Thank you for reading, and see you next time!

---

**Need Help?**
- Download: [GitHub Releases](https://github.com/ytxerp/ytx-client/releases)
- Documentation: [User Tutorials](../README.md#user-tutorial)
- Support: [Buy Me a Coffee](https://buymeacoffee.com/ytx.cash)
