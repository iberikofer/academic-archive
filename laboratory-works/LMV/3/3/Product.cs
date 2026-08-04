public class Product
{
    public string Name { get; set; }
    public double Price { get; set; }
    public string AsciiImage { get; set; }

    public Product(string name, double price, string ascii)
    {
        Name = name;
        Price = price;
        AsciiImage = ascii;
    }
}    