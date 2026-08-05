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
    public partial class FormAdmin : Form
    {
        public FormAdmin()
        {
            InitializeComponent();
        }

        public string NewName => txtAdminName.Text;
        public double NewPrice => double.TryParse(txtAdminPrice.Text, out double p) ? p : 0;
        public string NewAscii => txtAdminAscii.Text;

        private void btnAddAdmin_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(txtAdminName.Text))
            {
                MessageBox.Show("Please enter a product name!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            this.DialogResult = DialogResult.OK;
            this.Close();
        }

        private void txtAdminPrice_KeyPress(object sender, KeyPressEventArgs e)
        {
            if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar) && (e.KeyChar != '.'))
            {
                e.Handled = true;
            }
        }

        private void btnAddProduct_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrWhiteSpace(txtAdminName.Text) ||
                string.IsNullOrWhiteSpace(txtAdminPrice.Text) ||
                string.IsNullOrWhiteSpace(txtAdminAscii.Text))
            {
                MessageBox.Show("Please fill in all fields!", "Validation Error",
                                MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

            if (!double.TryParse(txtAdminPrice.Text, out _))
            {
                MessageBox.Show("Please enter a valid numeric price!", "Type Error",
                                MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }

                this.DialogResult = DialogResult.OK;
                this.Close();
        }

        private void btnBack_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Cancel;
            this.Close(); 
        }
    }
}
