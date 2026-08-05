import React from "react";

export default function CartView({ cart, onRemove, onCheckout }) {
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
