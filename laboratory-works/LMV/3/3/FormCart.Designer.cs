namespace _3
{
    partial class FormCart
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
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
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            listCartItems = new ListBox();
            lblTotal = new Label();
            btnCheckout = new Button();
            SuspendLayout();
            // 
            // listCartItems
            // 
            listCartItems.Dock = DockStyle.Top;
            listCartItems.FormattingEnabled = true;
            listCartItems.Location = new Point(0, 0);
            listCartItems.Name = "listCartItems";
            listCartItems.Size = new Size(782, 344);
            listCartItems.TabIndex = 0;
            listCartItems.DoubleClick += listCartItems_DoubleClick;
            // 
            // lblTotal
            // 
            lblTotal.AutoSize = true;
            lblTotal.Location = new Point(324, 370);
            lblTotal.Name = "lblTotal";
            lblTotal.Size = new Size(49, 20);
            lblTotal.TabIndex = 1;
            lblTotal.Text = "Total: ";
            // 
            // btnCheckout
            // 
            btnCheckout.Location = new Point(324, 409);
            btnCheckout.Name = "btnCheckout";
            btnCheckout.Size = new Size(137, 33);
            btnCheckout.TabIndex = 2;
            btnCheckout.Text = "Pay and Checkout";
            btnCheckout.UseVisualStyleBackColor = true;
            btnCheckout.Click += btnCheckout_Click;
            // 
            // FormCart
            // 
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(782, 453);
            Controls.Add(btnCheckout);
            Controls.Add(lblTotal);
            Controls.Add(listCartItems);
            MaximumSize = new Size(800, 500);
            MinimumSize = new Size(800, 500);
            Name = "FormCart";
            Text = "FormCart";
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private ListBox listCartItems;
        private Label lblTotal;
        private Button btnCheckout;
    }
}