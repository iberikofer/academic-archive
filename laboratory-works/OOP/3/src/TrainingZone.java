import java.util.Arrays;

public class TrainingZone {
    private String name;
    private Athlete[] zoneAthletes;

    public TrainingZone(String name) {
        this.name = name;
        this.zoneAthletes = new Athlete[0];
    }

    public String getName() {
        return name;
    }

    public Athlete[] getZoneAthletes() {
        return zoneAthletes;
    }

    public void setZoneAthletes(Athlete[] athletes) {
        this.zoneAthletes = athletes;
    }

    public void addAthlete(Athlete athlete) {
        zoneAthletes = Arrays.copyOf(zoneAthletes, zoneAthletes.length + 1);
        zoneAthletes[zoneAthletes.length - 1] = athlete;
    }

    public void removeAthlete(int index) {
        if (index >= 0 && index < zoneAthletes.length) {
            for (int i = index; i < zoneAthletes.length - 1; i++) {
                zoneAthletes[i] = zoneAthletes[i + 1];
            }
            zoneAthletes = Arrays.copyOf(zoneAthletes, zoneAthletes.length - 1);
        }
    }

    public void printZone() {
        System.out.println("=== ZONE: " + name + " ===");
        if (zoneAthletes.length == 0) {
            System.out.println("[This zone is empty]");
        } else {
            for (int i = 0; i < zoneAthletes.length; i++) {
                System.out.print("[" + i + "] ");
                zoneAthletes[i].athletePrint();
            }
        }
        System.out.println("=======================\n");
    }
}