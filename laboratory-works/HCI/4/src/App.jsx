import React, { useState, useEffect } from "react";
import { initialProducts } from "./data";
import "./App.css";

export default function App() {
  const [products, setProducts] = useState(() => {
    const saved = localStorage.getItem("cyber_products");
    return saved ? JSON.parse(saved) : initialProducts;
  });

  const [cart, setCart] = useState(() => {
    const saved = localStorage.getItem("cyber_cart");
    return saved ? JSON.parse(saved) : [];
  });

  const [user, setUser] = useState(() => {
    return localStorage.getItem("cyber_user") || null;
  });

  const [orderHistory, setOrderHistory] = useState(() => {
    const saved = localStorage.getItem("cyber_history");
    return saved ? JSON.parse(saved) : [];
  });

  const [view, setView] = useState("shop");
  const [searchQuery, setSearchQuery] = useState("");

  useEffect(() => {
    localStorage.setItem("cyber_products", JSON.stringify(products));
  }, [products]);

  useEffect(() => {
    localStorage.setItem("cyber_cart", JSON.stringify(cart));
  }, [cart]);

  useEffect(() => {
    localStorage.setItem("cyber_history", JSON.stringify(orderHistory));
  }, [orderHistory]);

  useEffect(() => {
    if (user) {
      localStorage.setItem("cyber_user", user);
    } else {
      localStorage.removeItem("cyber_user");
    }
  }, [user]);

  const addToCart = (product) => {
    if (!user) {
      alert("Please Sign in to add items to your cart!");
      return;
    }
    setCart([...cart, product]);
    alert(`${product.name} added to cart!`);
  };

  const removeFromCart = (indexToRemove) => {
    setCart(cart.filter((_, index) => index !== indexToRemove));
  };

  const checkout = () => {
    if (cart.length === 0) return;
    const total = cart.reduce((sum, item) => sum + item.price, 0);
    const orderRecord = `[${new Date().toLocaleTimeString()}] User: ${user} | Total: $${total} | Items: ${cart.map((i) => i.name).join(", ")}`;

    setOrderHistory([...orderHistory, orderRecord]);
    setCart([]);
    alert("Payment successful!");
    setView("shop");
  };

  const filteredProducts = products.filter((p) =>
    p.name.toLowerCase().includes(searchQuery.toLowerCase()),
  );

  return (
    <div className="app-container">
      <nav className="navbar">
        <div className="nav-group">
          <h1 className="nav-brand" onClick={() => setView("shop")}>
            ASCII CyberShop
          </h1>
          {user ? (
            <button onClick={() => setUser(null)} className="btn btn-light">
              Logout ({user})
            </button>
          ) : (
            <button
              onClick={() => setView("register")}
              className="btn btn-light">
              Sign in
            </button>
          )}
          <button onClick={() => setView("admin")} className="btn btn-dark">
            Admin
          </button>
        </div>

        {view === "shop" && (
          <input
            type="text"
            placeholder="Search tovar..."
            className="nav-input"
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
        )}

        <button onClick={() => setView("cart")} className="btn btn-green">
          Cart ({cart.length})
        </button>
      </nav>

      <main className="main-content">
        {view === "shop" && (
          <div className="product-grid">
            {filteredProducts.map((prod) => (
              <ProductCard
                key={prod.id}
                product={prod}
                onAdd={() => addToCart(prod)}
              />
            ))}
          </div>
        )}
        {view === "register" && (
          <RegisterView
            onRegister={(name) => {
              setUser(name);
              setView("shop");
            }}
          />
        )}
        {view === "cart" && (
          <CartView
            cart={cart}
            onRemove={removeFromCart}
            onCheckout={checkout}
          />
        )}
        {view === "admin" && (
          <AdminView
            onAddProduct={(newProd) =>
              setProducts([...products, { ...newProd, id: Date.now() }])
            }
            orders={orderHistory}
          />
        )}
      </main>
    </div>
  );
}

function ProductCard({ product, onAdd }) {
  return (
    <div className="product-card">
      <pre className="ascii-container">{product.ascii}</pre>
      <hr />
      <div className="product-info">
        <h3>{product.name}</h3>
        <p className="price">Price: ${product.price}</p>
      </div>
      <button onClick={onAdd} className="btn btn-blue">
        Add to Cart
      </button>
    </div>
  );
}

function RegisterView({ onRegister }) {
  const [username, setUsername] = useState("");

  const handleSubmit = (e) => {
    e.preventDefault();
    if (!username.trim()) return alert("Please enter a username");
    onRegister(username);
  };

  return (
    <div className="panel panel-sm" style={{ textAlign: "center" }}>
      <h2>Create an account please</h2>
      <form onSubmit={handleSubmit}>
        <div className="form-group">
          <label>Your username</label>
          <input
            type="text"
            className="form-input"
            value={username}
            onChange={(e) => setUsername(e.target.value)}
          />
        </div>
        <div className="form-group">
          <label>Your password</label>
          <input type="password" className="form-input" />
        </div>
        <button
          type="submit"
          className="btn btn-blue"
          style={{ marginTop: "1rem" }}>
          Sign in
        </button>
      </form>
    </div>
  );
}

function CartView({ cart, onRemove, onCheckout }) {
  const total = cart.reduce((sum, item) => sum + item.price, 0);

  return (
    <div className="panel panel-md">
      <h2>Your Cart</h2>
      <hr style={{ marginBottom: "1rem" }} />
      {cart.length === 0 ? (
        <p>Cart is empty</p>
      ) : (
        <ul className="cart-list">
          {cart.map((item, idx) => (
            <li key={idx} className="cart-item">
              <span>
                {item.name} - ${item.price}
              </span>
              <button onClick={() => onRemove(idx)} className="btn-remove">
                Remove
              </button>
            </li>
          ))}
        </ul>
      )}
      <div className="cart-total">
        <span>Total:</span>
        <span className="total-price">${total}</span>
      </div>
      <button
        onClick={onCheckout}
        className="btn btn-green"
        style={{ width: "100%", padding: "1rem" }}>
        Pay and Checkout
      </button>
    </div>
  );
}

function AdminView({ onAddProduct, orders }) {
  const [isLogged, setIsLogged] = useState(false);
  const [password, setPassword] = useState("");

  const [newName, setNewName] = useState("");
  const [newPrice, setNewPrice] = useState("");
  const [newAscii, setNewAscii] = useState("");

  // Новий стан для пошуку по історії
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

  // Фільтрація замовлень (пошук незалежний від регістру)
  const filteredOrders = orders.filter((o) =>
    o.toLowerCase().includes(historySearch.toLowerCase()),
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
