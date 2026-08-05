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
    public partial class FormDashboard : Form
    {
        public FormDashboard(List<string> history)
        {
            InitializeComponent();

            foreach (string order in history)
            {
                listOrders.Items.Add(order);
            }
        }

        private void btnAddNewProduct_Click(object sender, EventArgs e)
        {
            this.DialogResult = DialogResult.Yes;
            this.Close();
        }
    }
}
