public class Equipment implements Cloneable {
    String name;
    int value;
    String unit;

    public Equipment(String name, int value, String unit) {
        this.name = name;
        this.value = value;
        this.unit = unit;
    }

    @Override
    public Object clone() throws CloneNotSupportedException {
        return super.clone();
    }

    @Override
    public String toString() {
        return this.name + " " + this.value + this.unit + "\n";
    }
}