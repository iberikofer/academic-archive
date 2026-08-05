import React from "react";

export default function Navbar({
  view,
  setView,
  user,
  setUser,
  searchQuery,
  setSearchQuery,
  cartCount,
}) {
  return (
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
        Cart ({cartCount})
      </button>
    </nav>
  );
}
