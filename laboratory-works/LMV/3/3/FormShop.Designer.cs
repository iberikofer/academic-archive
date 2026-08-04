namespace _3
{
    partial class FormShop
    {
        /// <summary>
        ///  Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        ///  Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        ///  Required method for Designer support - do not modify
        ///  the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            flowLayoutPanel1 = new FlowLayoutPanel();
            hr = new Label();
            panel1 = new Panel();
            btnAdmin = new Button();
            btnCart = new Button();
            btnRegister = new Button();
            btnSearch = new Button();
            textBox1 = new TextBox();
            label1 = new Label();
            panel1.SuspendLayout();
            SuspendLayout();
            // 
            // flowLayoutPanel1
            // 
            flowLayoutPanel1.AutoScroll = true;
            flowLayoutPanel1.BackColor = SystemColors.AppWorkspace;
            flowLayoutPanel1.Dock = DockStyle.Fill;
            flowLayoutPanel1.Location = new Point(0, 103);
            flowLayoutPanel1.Name = "flowLayoutPanel1";
            flowLayoutPanel1.Size = new Size(982, 450);
            flowLayoutPanel1.TabIndex = 0;
            // 
            // hr
            // 
            hr.AutoSize = true;
            hr.Location = new Point(3, 103);
            hr.Name = "hr";
            hr.Size = new Size(789, 20);
            hr.TabIndex = 0;
            hr.Text = "----------------------------------------------------------------------------------------------------------------------------------";
            // 
            // panel1
            // 
            panel1.BackColor = SystemColors.HotTrack;
            panel1.Controls.Add(btnAdmin);
            panel1.Controls.Add(btnCart);
            panel1.Controls.Add(btnRegister);
            panel1.Controls.Add(hr);
            panel1.Controls.Add(btnSearch);
            panel1.Controls.Add(textBox1);
            panel1.Controls.Add(label1);
            panel1.Dock = DockStyle.Top;
            panel1.Location = new Point(0, 0);
            panel1.Name = "panel1";
            panel1.Size = new Size(982, 103);
            panel1.TabIndex = 2;
            // 
            // btnAdmin
            // 
            btnAdmin.Location = new Point(12, 47);
            btnAdmin.Name = "btnAdmin";
            btnAdmin.Size = new Size(94, 29);
            btnAdmin.TabIndex = 6;
            btnAdmin.Text = "Admin";
            btnAdmin.UseVisualStyleBackColor = true;
            btnAdmin.Click += btnAdmin_Click;
            // 
            // btnCart
            // 
            btnCart.Anchor = AnchorStyles.Top;
            btnCart.Cursor = Cursors.Hand;
            btnCart.Location = new Point(926, 12);
            btnCart.Name = "btnCart";
            btnCart.Size = new Size(44, 29);
            btnCart.TabIndex = 5;
            btnCart.Text = "Cart";
            btnCart.UseVisualStyleBackColor = true;
            btnCart.Click += btnCart_Click;
            // 
            // btnRegister
            // 
            btnRegister.Anchor = AnchorStyles.Top;
            btnRegister.Cursor = Cursors.Hand;
            btnRegister.Location = new Point(12, 12);
            btnRegister.Name = "btnRegister";
            btnRegister.Size = new Size(94, 29);
            btnRegister.TabIndex = 4;
            btnRegister.Text = "Sign in";
            btnRegister.UseVisualStyleBackColor = true;
            btnRegister.Click += btnRegister_Click;
            // 
            // btnSearch
            // 
            btnSearch.Anchor = AnchorStyles.Top;
            btnSearch.Cursor = Cursors.Hand;
            btnSearch.Location = new Point(635, 31);
            btnSearch.Name = "btnSearch";
            btnSearch.Size = new Size(98, 44);
            btnSearch.TabIndex = 1;
            btnSearch.Text = "Shukaty";
            btnSearch.UseVisualStyleBackColor = true;
            btnSearch.Click += btnSearch_Click;
            // 
            // textBox1
            // 
            textBox1.Anchor = AnchorStyles.Top;
            textBox1.Cursor = Cursors.IBeam;
            textBox1.Location = new Point(390, 40);
            textBox1.Name = "textBox1";
            textBox1.Size = new Size(228, 27);
            textBox1.TabIndex = 0;
            // 
            // label1
            // 
            label1.Anchor = AnchorStyles.Top;
            label1.AutoSize = true;
            label1.FlatStyle = FlatStyle.Flat;
            label1.Font = new Font("Segoe UI", 13.2000008F, FontStyle.Bold, GraphicsUnit.Point, 204);
            label1.ForeColor = SystemColors.ButtonFace;
            label1.Location = new Point(237, 36);
            label1.Name = "label1";
            label1.Size = new Size(147, 31);
            label1.TabIndex = 3;
            label1.Text = "Search tovar";
            // 
            // FormShop
            // 
            AcceptButton = btnSearch;
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            AutoScroll = true;
            ClientSize = new Size(982, 553);
            Controls.Add(flowLayoutPanel1);
            Controls.Add(panel1);
            MinimumSize = new Size(1000, 600);
            Name = "FormShop";
            Text = "ASCII CyberShop";
            panel1.ResumeLayout(false);
            panel1.PerformLayout();
            ResumeLayout(false);
        }

        #endregion

        private FlowLayoutPanel flowLayoutPanel1;
        private TextBox textBox1;
        private Button btnSearch;
        private Panel panel1;
        private Label label1;
        private Button btnRegister;
        private Label hr;
        private Button btnCart;
        private Button btnAdmin;
    }
}
