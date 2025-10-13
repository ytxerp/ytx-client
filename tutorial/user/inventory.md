# YTX Inventory

Hello everyone, today we'll introduce how **YTX** manages inventory.

The current software version is **0.2.7**, and it can be downloaded from the [Github](https://github.com/ytxerp/ytx-client/releases/tag/0.2.7).

---

## Main Interface

First, let's take a look at the main interface.

1. **Login**
   - Login requires **email**, **password**, and **workspace**.
   - Email and password are self-registered by users, while workspaces are created by administrators. After successful registration, users can apply to join a workspace, and then administrators set appropriate permissions and grant access to the data.
   - You can choose **whether to save password locally** for convenient future logins.

2. **Menu**
   - At the top is the **menu bar**, containing three groups: File, Edit, and Account.
   - Next is the **toolbar**, commonly used functions include: Insert Node, Append Node, Edit Node Name, Reset Node Color, Create Node Group, Append Record, Delete, Select All, Invert Selection, Clear Selection, Bill, Settlement.
   - On the left is the **section bar**, currently four sections are available: Finance, Tasks, Inventory, and Collaboration.

3. **Inventory**
   - Today we'll introduce Inventory. Inventory may have different names, such as item, product, warehouse, etc., but they all mean the same thing.
   - By default, the interface is empty and requires creating **nodes** to gradually build the structure.

---

## Attributes

Next, let's introduce the **three core attributes** of nodes.

- **Rule**
  Can be set to `DICD` or `DDCI`, which determines how the node balance is calculated.

- **Type**
  - **B, Branch Node**: Aggregates data from leaf nodes under it.
  - **L, Leaf Node**: Where data is ultimately entered.

- **Unit**
  - **INT, Internal Code**: Your company's product code.
  - **EXT, External Code**: Customer's purchase code or supplier's sales code. When creating orders, the customer's code will be automatically imported based on your own code.
  - **POS, Position**: Indicates where the product is stored.
  - In actual use, internal code and position are most commonly used; external code is used for mapping relationships and is added in the **Collaboration** section.

---

## Demonstration

Let's see how the Inventory section works through a simple example.

### Nodes

1. Add a product branch node
   - Shortcut: `Alt + P`
   - Name it `Apple`, Type `B`, Unit `INT`.

2. Append three product child nodes
   - Select `Apple`, press `Alt + P`.
   - Name them: `Phone`, `Computer`, and `Tablet`, Type `L`, Unit also `INT`.

3. Add a position branch node
   - Select `Apple`, press `Alt + N` to insert at the same level.
   - Name it `Position`, Type `B`, Unit set to `POS`.

4. Append three position child nodes
   - Select `Position`, press `Alt + P`.
   - Name them: `Box 1`, `Box 2`, and `Box 3`, Type `L`, Unit `POS`.

5. If a node is created incorrectly, it can be deleted directly.

At this point, the basic structure is complete. You can expand or collapse nodes, and drag nodes to adjust their positions.

### **Important Notes**

- When appending a node, it will append a child node under the selected branch node; if no node is selected, it will be appended to the root level — that is, a top-level node will be created.
- When inserting a node, it will insert a new node at the same level as the selected node; if no node is selected, the operation will be invalid.
- Once a node's **Type** and **Unit** are set, they cannot be modified. If entered incorrectly, you must delete and recreate it.
- Branch nodes will only aggregate data from leaf nodes with the same **Unit**.
- The unit of **Position** nodes must be accurate so that incorrect data won't be collected when creating orders in the Order section.

---

### Records

Next is data entry.

1. **Adding**
   - Double-click `Apple - Phone` to enter the operation page.
   - The toolbar will automatically highlight available functions.

2. **Attributes**
   - **D**: Used to add related documents. Only the path is saved here, not the actual file.
   - **S**: Represents status, commonly used for verification.
   - **Association**: Can be associated with **Position** or **Other Products**.
     - If associated with position, it indicates storage location.
     - If associated with product, it can be understood as the product entering the next stage.
   - **Debit, Credit**:
     - Debit and credit are used to record data and always appear in pairs.
     - When the current node has data in debit, the associated node must have the same data in credit; when the current node has data in credit, the associated node must also have the same data in debit.
   - **Balance**:
     - If node rule is `DICD`, Balance = Debit − Credit.
     - If node rule is `DDCI`, Balance = Credit − Debit.

3. **Example**
   - Create a new record: Press `Ctrl + N`.
   - Set association to `Position - Box 3`.
   - Enter 50 for debit, 7749 for cost.
   - Use the `Ctrl + J` shortcut to quickly jump to the associated node's page.
   - Double-click the title to return to the Inventory section, and you'll see that the data just entered has been synchronized to the node.

4. **Viewing**
   - Data can be queried from two directions
   - In `Box 3`, view what specific items are inside.
   - In `Phone`, view which locations it's stored in.

5. **Search**
   - `Ctrl + F` allows quick searching by **node name** or **record description**.

---

## Summary

That's the introduction to **Inventory**. After understanding the logic of nodes and records, you can easily design different structures to adapt to actual needs, such as registering household items or company inventory.

That's all for today's introduction. We hope YTX can be helpful to you. See you next time.
