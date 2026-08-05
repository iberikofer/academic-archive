namespace _3
{
    public class Product
    {
        public string Name { get; set; }
        public decimal Price { get; set; }
        public string AsciiImage { get; set; }

        public Product(string name, decimal price, string ascii)
        {
            Name = name;
            Price = price;
            AsciiImage = ascii;
        }
    }
}