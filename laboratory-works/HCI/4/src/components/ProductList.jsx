import React from "react";
import ProductCard from "./ProductCard";

export default function ProductList({ products, onAddToCart }) {
  if (products.length === 0) {
    return (
      <div className="panel panel-md" style={{ textAlign: "center" }}>
        <p>No products found matching your search.</p>
      </div>
    );
  }

  return (
    <div className="product-grid">
      {products.map((prod) => (
        <ProductCard
          key={prod.id}
          product={prod}
          onAdd={() => onAddToCart(prod)}
        />
      ))}
    </div>
  );
}
