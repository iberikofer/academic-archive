import React, { useState } from "react";

export default function RegisterView({ onRegister }) {
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
