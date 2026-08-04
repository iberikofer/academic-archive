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
    public partial class FormAdminLogin : Form
    {
        public FormAdminLogin()
        {
            InitializeComponent();
        }

        private void btnLogin_Click(object sender, EventArgs e)
        {
            if (txtPassword.Text == "cyber_admin_2026")
            {
                this.DialogResult = DialogResult.OK;
                this.Close();
            }
            else
            {
                System.Media.SystemSounds.Hand.Play();
                MessageBox.Show("Wrong password!", "Security", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
        }
    }
}
