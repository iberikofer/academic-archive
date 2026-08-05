import java.util.Objects;

public class Athlete implements Cloneable, Comparable<Athlete> {
    private String name;
    private int age;
    private double weight;
    private double energyLvl;
    private Equipment equipment;

    public String getName() {
        return name;
    }
    public void setName(String name) {
        this.name = name;
    }
    public int getAge() {
        return age;
    }
    public void setAge(int age) {
        this.age = age;
    }
    public double getWeight() {
        return weight;
    }
    public void setWeight(double weight) {
        this.weight = weight;
    }
    public double getEnergyLvl() {
        return energyLvl;
    }
    public void setEnergyLvl(double energyLvl) {
        this.energyLvl = energyLvl;
    }
    public Equipment getEquipment() {
        return equipment;
    }
    public void setEquipment(Equipment equipment) {
        this.equipment = equipment;
    }

    public Athlete() {
        this("Gym bro", 20, 100.0, 100.0, new Equipment("Exercise band", 5, "kg"));
    }

    public Athlete(String name, int age, double weight, double energyLvl, Equipment equipment) {
        this.name = name;
        this.age = age;
        this.weight = weight;
        this.energyLvl = energyLvl;
        this.equipment = equipment;
    }

    public Athlete(String name) {
        this.name = name;
        this.equipment = null;
    }

    public void train() {
        if (this.energyLvl >= 5) {
            this.energyLvl -= 5;

            double weightLoss = java.util.concurrent.ThreadLocalRandom.current().nextDouble(5, 10);
            this.weight -= weightLoss;

            System.out.println(name + " has trained. Energy: " + energyLvl + "%, Weight: " + String.format("%.2f", weight));
        } else {
            System.out.println(name + " is too exhausted. Energy: " + energyLvl);
        }
    }

    public void eat() {
        if (this.energyLvl < 100) {
            this.energyLvl = Math.min(this.energyLvl + 2.5, 100.0);

            double weightGain = java.util.concurrent.ThreadLocalRandom.current().nextDouble(1, 5);
            this.weight += weightGain;

            System.out.println(name + " has recovered. Energy: " + energyLvl + "%, Weight: " + String.format("%.2f", weight));
        } else {
            System.out.println(name + " has fully recovered. Energy: !" + energyLvl);
        }
    }

    public void rest() {
        eat();
    }

    public void takeSelfie() {
        if (this.energyLvl < 5) {
            System.out.println(name + " is too exhausted to take selfie. Energy: " + energyLvl);
        } else {
            this.energyLvl -= 1.0;
            System.out.println(name + " looks pumped up. Energy: !" + energyLvl);
        }
    }

    public void athletePrint() {
        System.out.printf("=== Name: %s ===\n", name);
        if (getEquipment() != null) {
            System.out.printf("Age: %d years old | Weight: %.2f kg | Energy: %.1f%% | Equipment: %s\n",
                    age, weight, energyLvl, equipment.toString());
        } else {
            System.out.printf("Age: %d years old | Weight: %.2f kg | Energy: %.1f%% | Equipment: None\n",
                    age, weight, energyLvl);
        }
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) return true;
        if (obj == null || getClass() != obj.getClass()) return false;
        Athlete other = (Athlete) obj;
        return age == other.age &&
                Double.compare(other.weight, weight) == 0 &&
                Objects.equals(name, other.name);
    }

    @Override
    public int hashCode() {
        return Objects.hash(name, age, weight);
    }

    @Override
    public int compareTo(Athlete other) {
        if (other == null || other.name == null) return 1;
        if (this.name == null) return -1;
        return this.name.compareToIgnoreCase(other.name);
    }

    @Override
    public Object clone() throws CloneNotSupportedException {
        Athlete cloned = (Athlete) super.clone();
        if (this.equipment != null) {
            cloned.setEquipment((Equipment) this.equipment.clone());
        }
        return cloned;
    }

    public static class AgeComparator implements java.util.Comparator<Athlete> {
        @Override
        public int compare(Athlete a1, Athlete a2) {
            return Integer.compare(a1.getAge(), a2.getAge());
        }
    }

    @Override
    public String toString() {
        return "Age: " + age + " years old | Weight: " + weight + " kg | Energy: " + energyLvl + "% | Equipment: " + (equipment != null ? equipment.toString() : "None") + "\n";
    }
}