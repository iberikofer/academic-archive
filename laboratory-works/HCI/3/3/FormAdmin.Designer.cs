namespace _3
{
    partial class FormAdmin
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
            label1 = new Label();
            txtAdminName = new TextBox();
            label2 = new Label();
            label3 = new Label();
            txtAdminPrice = new TextBox();
            label4 = new Label();
            txtAdminAscii = new TextBox();
            btnAddProd = new Button();
            SuspendLayout();
            // 
            // label1
            // 
            label1.AutoSize = true;
            label1.Font = new Font("Segoe UI", 22F, FontStyle.Bold);
            label1.Location = new Point(205, 9);
            label1.Name = "label1";
            label1.Size = new Size(410, 50);
            label1.TabIndex = 0;
            label1.Text = "Adding a new product";
            // 
            // txtAdminName
            // 
            txtAdminName.Location = new Point(369, 85);
            txtAdminName.MaxLength = 20;
            txtAdminName.Name = "txtAdminName";
            txtAdminName.Size = new Size(246, 27);
            txtAdminName.TabIndex = 1;
            // 
            // label2
            // 
            label2.AutoSize = true;
            label2.Font = new Font("Segoe UI", 11F);
            label2.Location = new Point(205, 84);
            label2.Name = "label2";
            label2.Size = new Size(133, 25);
            label2.TabIndex = 2;
            label2.Text = "Product Name";
            // 
            // label3
            // 
            label3.AutoSize = true;
            label3.Font = new Font("Segoe UI", 11F);
            label3.Location = new Point(205, 126);
            label3.Name = "label3";
            label3.Size = new Size(125, 25);
            label3.TabIndex = 4;
            label3.Text = "Product Price";
            // 
            // txtAdminPrice
            // 
            txtAdminPrice.Location = new Point(369, 127);
            txtAdminPrice.MaxLength = 10;
            txtAdminPrice.Name = "txtAdminPrice";
            txtAdminPrice.Size = new Size(246, 27);
            txtAdminPrice.TabIndex = 3;
            txtAdminPrice.KeyPress += txtAdminPrice_KeyPress;
            // 
            // label4
            // 
            label4.AutoSize = true;
            label4.Font = new Font("Segoe UI", 11F);
            label4.Location = new Point(205, 243);
            label4.Name = "label4";
            label4.Size = new Size(142, 25);
            label4.TabIndex = 6;
            label4.Text = "Product Picture";
            // 
            // txtAdminAscii
            // 
            txtAdminAscii.Font = new Font("Consolas", 9F, FontStyle.Regular, GraphicsUnit.Point, 204);
            txtAdminAscii.Location = new Point(369, 170);
            txtAdminAscii.MaxLength = 50;
            txtAdminAscii.Multiline = true;
            txtAdminAscii.Name = "txtAdminAscii";
            txtAdminAscii.Size = new Size(246, 164);
            txtAdminAscii.TabIndex = 5;
            // 
            // btnAddProd
            // 
            btnAddProd.Location = new Point(355, 395);
            btnAddProd.Name = "btnAddProd";
            btnAddProd.Size = new Size(137, 43);
            btnAddProd.TabIndex = 7;
            btnAddProd.Text = "Add product";
            btnAddProd.UseVisualStyleBackColor = true;
            btnAddProd.Click += btnAddProduct_Click;
            // 
            // FormAdmin
            // 
            AcceptButton = btnAddProd;
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            BackColor = SystemColors.ActiveCaption;
            ClientSize = new Size(782, 453);
            Controls.Add(btnAddProd);
            Controls.Add(label4);
            Controls.Add(txtAdminAscii);
            Controls.Add(label3);
            Controls.Add(txtAdminPrice);
            Controls.Add(label2);
            Controls.Add(txtAdminName);
            Controls.Add(label1);
            MaximumSize = new Size(800, 500);
            MinimumSize = new Size(800, 500);
            Name = "FormAdmin";
            Text = "Admin Panel";
            ResumeLayout(false);
            PerformLayout();
        }

        #endregion

        private Label label1;
        private TextBox txtAdminName;
        private Label label2;
        private Label label3;
        private TextBox txtAdminPrice;
        private Label label4;
        private TextBox txtAdminAscii;
        private Button btnAddProd;
    }
}