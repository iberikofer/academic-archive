import React, { useState } from "react";

export default function AdminView({ onAddProduct, orders }) {
  const [isLogged, setIsLogged] = useState(false);
  const [password, setPassword] = useState("");

  const [newName, setNewName] = useState("");
  const [newPrice, setNewPrice] = useState("");
  const [newAscii, setNewAscii] = useState("");

  const [historySearch, setHistorySearch] = useState("");

  const handleLogin = (e) => {
    e.preventDefault();
    if (password === "2026") setIsLogged(true);
    else alert("Wrong password!");
  };

  const handleAdd = (e) => {
    e.preventDefault();
    if (!newName || !newPrice || !newAscii) return alert("Fill all fields");
    onAddProduct({
      name: newName,
      price: parseFloat(newPrice),
      ascii: newAscii,
    });
    setNewName("");
    setNewPrice("");
    setNewAscii("");
    alert("Product added successfully!");
  };

  const filteredOrders = orders.filter((o) =>
    o.toLowerCase().includes(historySearch.toLowerCase())
  );

  if (!isLogged) {
    return (
      <div className="panel panel-sm" style={{ textAlign: "center" }}>
        <h2>Type Admin password</h2>
        <form onSubmit={handleLogin}>
          <div className="form-group">
            <input
              type="password"
              value={password}
              onChange={(e) => setPassword(e.target.value)}
              className="form-input"
            />
          </div>
          <button type="submit" className="btn btn-dark">
            Log in
          </button>
        </form>
      </div>
    );
  }

  return (
    <div>
      <h2 className="admin-header">Admin Panel</h2>
      <div className="dashboard-grid">
        <div className="panel">
          <h2>Adding a new product</h2>
          <form onSubmit={handleAdd}>
            <div className="form-group">
              <input
                placeholder="Product Name"
                className="form-input"
                value={newName}
                onChange={(e) => setNewName(e.target.value)}
              />
            </div>
            <div className="form-group">
              <input
                type="number"
                placeholder="Product Price"
                className="form-input"
                value={newPrice}
                onChange={(e) => setNewPrice(e.target.value)}
              />
            </div>
            <div className="form-group">
              <textarea
                placeholder="Product Picture (ASCII)"
                className="form-textarea"
                value={newAscii}
                onChange={(e) => setNewAscii(e.target.value)}
              />
            </div>
            <button type="submit" className="btn btn-blue">
              Add product
            </button>
          </form>
        </div>

        <div className="panel flex flex-col">
          <h2>Order History (Dashboard)</h2>

          <input
            type="text"
            placeholder="Search by username..."
            className="form-input search-history-input"
            value={historySearch}
            onChange={(e) => setHistorySearch(e.target.value)}
          />

          <div className="history-box">
            {filteredOrders.length === 0
              ? "No orders found."
              : filteredOrders.map((o, i) => (
                  <div key={i} className="history-item">
                    {o}
                  </div>
                ))}
          </div>
        </div>
      </div>
    </div>
  );
}
