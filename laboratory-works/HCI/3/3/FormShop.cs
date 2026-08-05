using System.Linq;

namespace _3
{
    public partial class FormShop : Form
    {
        bool isLoggedIn = false;

        private Panel CreateProductCard(Product prod)
        {
            Panel card = new Panel { Size = new Size(220, 380), BorderStyle = BorderStyle.FixedSingle, Margin = new Padding(10) };

            Label lblAscii = new Label
            {
                Text = prod.AsciiImage,
                Font = new Font("Courier New", 8),
                Dock = DockStyle.Top,
                Height = 230,
                TextAlign = ContentAlignment.MiddleCenter,
                BackColor = Color.White
            };

            Panel separator = new Panel
            {
                BackColor = Color.Gray,
                Dock = DockStyle.Top,
                Height = 1,
                Margin = new Padding(5, 0, 5, 0)
            };

            Label lblInfo = new Label
            {
                Text = $"\n{prod.Name}\nPrice: {prod.Price}$",
                Font = new Font("Segoe UI", 9, FontStyle.Bold),
                Dock = DockStyle.Fill,
                TextAlign = ContentAlignment.TopCenter,
                BackColor = Color.White
            };

            Button btnBuy = new Button { Text = "Add to Cart", Dock = DockStyle.Bottom, Height = 40 };
            btnBuy.Click += (s, ev) =>
            {
                if (!isLoggedIn)
                {
                    MessageBox.Show("Please Sign in to add items to your cart!", "Account Required",
                                    MessageBoxButtons.OK, MessageBoxIcon.Error);
                }
                else
                {
                    cart.Add(prod);
                    MessageBox.Show($"{prod.Name} added to cart!", "Cart Updated", MessageBoxButtons.OK, MessageBoxIcon.Information);
                }
            };

            card.Controls.Add(lblInfo);
            card.Controls.Add(separator);
            card.Controls.Add(lblAscii);
            card.Controls.Add(btnBuy);
            return card;
        }

        void ShowProducts()
        {
            flowLayoutPanel1.Controls.Clear();
            foreach (var prod in catalog)
            {
                flowLayoutPanel1.Controls.Add(CreateProductCard(prod));
            }
        }

        private List<Product> catalog = new List<Product>();
        private List<Product> cart = new List<Product>();
        public List<string> OrderHistory { get; private set; } = new List<string>(); 
        public string CurrentUser { get; set; } = "Guest";
        public FormShop()
        {
            InitializeComponent();
            btnCart.Visible = false;

            catalog.Add(new Product("Cyber Laptop", 1500m,
                "  __________  \n" +
                " |          | \n" +
                " |   ASCII  | \n" +
                " |__________| \n" +
                " /QWERTYUIOP\\ \n" +
                " /____________\\ "));

            catalog.Add(new Product("Retro Phone", 500m,
                "  .----.  \n" +
                "  | =) |  \n" +
                "  |____|  \n" +
                "  |::::|  \n" +
                "  '----'  "));

            catalog.Add(new Product("Cyber Sword", 750m,
                "      /\\      \n" +
                "     /  \\     \n" +
                "    /  \\ \\    \n" +
                "    |  | |    \n" +
                "    |  | |    \n" +
                "    |  | |    \n" +
                "    |  | |    \n" +
                "    |  | |    \n" +
                "    |____|    \n" +
                "      ||      \n" +
                "      ||      \n" +
                "      I__I      "));

            catalog.Add(new Product("Neural Implant", 3200m,
                "    ______    \n" +
                "    /      \\    \n" +
                "   | [INC] |   \n" +
                "    \\______/    \n" +
                "     /    \\     "));

            catalog.Add(new Product("AI robot", 2100m,
                "    .----.    \n" +
                "  /  _  _  \\  \n" +
                " (  (o)(o)  ) \n" +
                "  \\   w   /  \n" +
                "   '------'   "));

            catalog.Add(new Product("ASCII Drone", 1200m,
                " [o]------[o] \n" +
                "     \\  /     \n" +
                "      ][      \n" +
                "     /  \\     \n" +
                " [o]------[o] "));

            catalog.Add(new Product("Holowatch", 450m,
                "    .----.    \n" +
                "   |  14  |   \n" +
                "   |  ::  |   \n" +
                "   |  88  |   \n" +
                "    '----'    "));

            catalog.Add(new Product("Retro Console", 300m,
                "  _________  \n" +
                " |  _   _  | \n" +
                " | | | | | | \n" +
                " | |0| |1| | \n" +
                " | |1| |0| | \n" +
                " | |0| |1| | \n" +
                " | |1| |0| | \n" +
                " | |0| |1| | \n" +
                " | |_| |_| | \n" +
                " |_________| "));

            catalog.Add(new Product("VR Goggles", 800m,
                "   .-------.   \n" +
                "  /   VR    \\  \n" +
                " |  [=---=]  | \n" +
                "  \\_________/  "));

            catalog.Add(new Product("Cyber Eye", 1200m,
                "    .----.    \n" +
                "   /  __  \\   \n" +
                "  | ( * ) |  \n" +
                "   \\  --  /   \n" +
                "    '----'    "));

            catalog.Add(new Product("Data Core", 3500m,
                "  ________  \n" +
                "  |       |  \n" +
                "  |  / \\  |  \n" +
                "  | < O > |  \n" +
                "  |  \\ /  |  \n" +
                "   |_______|   "));

            catalog.Add(new Product("Plasma Pistol", 2500m,
                "            _    \n" +
                "___________||___\n" +
                "  |_|_|_|_|_|_|____\\  \n" +
                "            | |     \n" +
                "            |_|     "));

            catalog.Add(new Product("Energy Crystal", 1800m,
                "     / \\     \n" +
                "    | o |    \n" +
                "    | o |    \n" +
                "     \\ /     ")); 

            catalog.Add(new Product("Nano-Bot", 600m,
                "     _o_      \n" +
                "    / | \\     \n" +
                "    \\_|_/     \n" +
                "     ' '      "));

            catalog.Add(new Product("Hover Disc", 2200m,
                "   .------.   \n" +
                "   /        \\   \n" +
                "  (  #####  )  \n" +
                "  \\        /  \n" +
                "   '-------'   \n" +
                "    ^  ^  ^    \n" +
                "    ------------------    "));

            catalog.Add(new Product("Smart Helmet", 1700m,
                "     .---.     \n" +
                "    / / \\ \\    \n" +
                "   | [---] |   \n" +
                "    \\_____/    "));

            ShowProducts();
        }

        private void btnSearch_Click(object sender, EventArgs e)
        {
            string searchText = textBox1.Text.ToLower();
            flowLayoutPanel1.Controls.Clear();

            foreach (var prod in catalog)
            {
                if (prod.Name.ToLower().Contains(searchText))
                {
                    flowLayoutPanel1.Controls.Add(CreateProductCard(prod));
                }
            }
        }

        private void btnRegister_Click(object sender, EventArgs e)
        {
            if (btnRegister.Text.Contains("Logout"))
            {
                isLoggedIn = false;
                btnRegister.Text = "Sign in";
                btnCart.Visible = false;
                cart.Clear();
                MessageBox.Show("You have been logged out.", "Account status", MessageBoxButtons.OK, MessageBoxIcon.Information);
                CurrentUser = "Guest";
                return;
            }

            FormRegister regForm = new FormRegister();
            if (regForm.ShowDialog() == DialogResult.OK)
            {
                string userName = regForm.Controls["txtName"]?.Text ?? "User";
                isLoggedIn = true;
                btnRegister.Width = 150;
                btnRegister.Text = "Logout: " + userName;
                btnCart.Visible = true;
                MessageBox.Show("Welcome, " + userName + "!", "Success", MessageBoxButtons.OK, MessageBoxIcon.Information);
                CurrentUser = userName;
            }
        }

        private void btnCart_Click(object sender, EventArgs e)
        {
            if (cart.Count == 0)
            {
                MessageBox.Show("Your shopping cart is empty!", "Cart",
                                MessageBoxButtons.OK, MessageBoxIcon.Information);
                return;
            }

            FormCart cartWindow = new FormCart(cart, CurrentUser, OrderHistory);
            cartWindow.ShowDialog();

            ShowProducts();
        }

        private void btnAdmin_Click(object sender, EventArgs e)
        {
            FormAdminLogin login = new FormAdminLogin();
            if (login.ShowDialog() == DialogResult.OK)
            {
                bool stayInDashboard = true;

                while (stayInDashboard)
                {
                    FormDashboard dashboard = new FormDashboard(OrderHistory);
                    DialogResult dashResult = dashboard.ShowDialog();

                    if (dashResult == DialogResult.Yes)
                    {
                        FormAdmin addProductForm = new FormAdmin();
                        if (addProductForm.ShowDialog() == DialogResult.OK)
                        {
                            catalog.Add(new Product(addProductForm.NewName, addProductForm.NewPrice, addProductForm.NewAscii));
                            ShowProducts(); 

                            System.Media.SystemSounds.Asterisk.Play();
                            MessageBox.Show("New product added successfully!", "Success", MessageBoxButtons.OK, MessageBoxIcon.Information);
                        }
                    }
                    else
                    {
                        stayInDashboard = false;
                    }
                }
            }
        }
    }
}
