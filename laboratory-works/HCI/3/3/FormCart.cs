using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace _3
{
    public partial class FormCart : Form
    {
        private List<Product> _cart;
        private string _userName;
        private List<string> _history;

        public FormCart(List<Product> cart, string userName, List<string> history)
        {
            InitializeComponent();
            _cart = cart;
            _userName = userName;
            _history = history;
            UpdateList();
        }

        private void UpdateList()
        {
            listCartItems.Items.Clear();

            listCartItems.Font = new Font("Consolas", 10);

            var grouped = _cart.GroupBy(p => p.Name).Select(g => new
            {
                Name = g.Key,
                Qty = g.Count(),
                Price = g.First().Price,
                Total = g.Sum(p => p.Price)
            });

            foreach (var item in grouped)
            {
                listCartItems.Items.Add($"{item.Name,-20} (${item.Price}) x{item.Qty} = {item.Total}$   (Double-click to remove)");
            }

            decimal totalSum = _cart.Sum(p => p.Price);
            lblTotal.Text = $"Total Amount: {totalSum}$";
            lblTotal.ForeColor = Color.DarkBlue;
        }

        private void listCartItems_SelectedIndexChanged(object sender, EventArgs e)
        {
            if (listCartItems.SelectedItem is string selectedLine)
            {
                string prodName = selectedLine.Split('(')[0].Trim();

                var itemToRemove = _cart.FirstOrDefault(p => p.Name == prodName);
                if (itemToRemove != null)
                {
                    _cart.Remove(itemToRemove);
                    System.Media.SystemSounds.Beep.Play();
                    UpdateList();
                }
            }
        }

        private void btnCheckout_Click(object sender, EventArgs e)
        {
            DialogResult res = MessageBox.Show("Do you want to complete the purchase?", "Checkout",
                                               MessageBoxButtons.YesNo, MessageBoxIcon.Question);

            if (res == DialogResult.Yes)
            {
                string items = string.Join(", ", _cart.Select(p => p.Name));
                decimal total = _cart.Sum(p => p.Price);
                string orderRecord = $"[{DateTime.Now.ToShortTimeString()}] User: {_userName} | Total: {total}$ | Items: {items}";

                _history.Add(orderRecord);

                System.Media.SystemSounds.Exclamation.Play();
                MessageBox.Show("Payment successful!", "Success", MessageBoxButtons.OK, MessageBoxIcon.Information);

                _cart.Clear();
                this.Close();
            }
        }

        private void listCartItems_DoubleClick(object sender, EventArgs e)
        {
            if (listCartItems.SelectedItem is string selectedLine)
            {
                string prodName = selectedLine.Split('(')[0].Trim();

                DialogResult res = MessageBox.Show($"Remove '{prodName}' from cart?", "Confirm",
                                                  MessageBoxButtons.YesNo, MessageBoxIcon.Warning);

                if (res == DialogResult.Yes)
                {
                    var itemToRemove = _cart.FirstOrDefault(p => p.Name == prodName);
                    if (itemToRemove != null)
                    {
                        _cart.Remove(itemToRemove);
                        System.Media.SystemSounds.Beep.Play(); 
                        UpdateList();  
                    }
                }
            }
        }
    }
}
