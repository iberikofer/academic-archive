import React, { useState, useEffect } from "react";
import { initialProducts } from "./data";
import Navbar from "./components/Navbar";
import ProductList from "./components/ProductList";
import RegisterView from "./components/RegisterView";
import CartView from "./components/CartView";
import AdminView from "./components/AdminView";
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
    setCart((prevCart) => [...prevCart, product]);
    alert(`${product.name} added to cart!`);
  };

  const removeFromCart = (indexToRemove) => {
    setCart((prevCart) => prevCart.filter((_, index) => index !== indexToRemove));
  };

  const checkout = () => {
    if (cart.length === 0) return;
    const total = cart.reduce((sum, item) => sum + item.price, 0);
    const orderRecord = `[${new Date().toLocaleTimeString()}] User: ${user} | Total: $${total} | Items: ${cart
      .map((i) => i.name)
      .join(", ")}`;

    setOrderHistory((prev) => [...prev, orderRecord]);
    setCart([]);
    alert("Payment successful!");
    setView("shop");
  };

  const handleAddProduct = (newProd) => {
    setProducts((prev) => [...prev, { ...newProd, id: Date.now() }]);
  };

  const filteredProducts = products.filter((p) =>
    p.name.toLowerCase().includes(searchQuery.toLowerCase())
  );

  return (
    <div className="app-container">
      <Navbar
        view={view}
        setView={setView}
        user={user}
        setUser={setUser}
        searchQuery={searchQuery}
        setSearchQuery={setSearchQuery}
        cartCount={cart.length}
      />

      <main className="main-content">
        {view === "shop" && (
          <ProductList
            products={filteredProducts}
            onAddToCart={addToCart}
          />
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
            onAddProduct={handleAddProduct}
            orders={orderHistory}
          />
        )}
      </main>
    </div>
  );
}
