namespace _3
{
    partial class FormDashboard
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
            listOrders = new ListBox();
            button1 = new Button();
            SuspendLayout();
            // 
            // listOrders
            // 
            listOrders.Dock = DockStyle.Top;
            listOrders.FormattingEnabled = true;
            listOrders.Location = new Point(0, 0);
            listOrders.Name = "listOrders";
            listOrders.Size = new Size(782, 364);
            listOrders.TabIndex = 0;
            // 
            // button1
            // 
            button1.Location = new Point(310, 380);
            button1.Name = "button1";
            button1.Size = new Size(180, 51);
            button1.TabIndex = 1;
            button1.Text = "Add a new product";
            button1.UseVisualStyleBackColor = true;
            button1.Click += btnAddNewProduct_Click;
            // 
            // FormDashboard
            // 
            AutoScaleDimensions = new SizeF(8F, 20F);
            AutoScaleMode = AutoScaleMode.Font;
            ClientSize = new Size(782, 453);
            Controls.Add(button1);
            Controls.Add(listOrders);
            Name = "FormDashboard";
            Text = "FormDashboard";
            ResumeLayout(false);
        }

        #endregion

        private ListBox listOrders;
        private Button button1;
    }
}