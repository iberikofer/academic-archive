import React from "react";

export default function ProductCard({ product, onAdd }) {
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
