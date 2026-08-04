package gymworld;

import javafx.scene.Group;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.input.MouseButton;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.scene.text.Text;
import javafx.scene.effect.DropShadow;
import javafx.animation.StrokeTransition;
import javafx.animation.Timeline;
import javafx.util.Duration;
import javafx.animation.SequentialTransition;

import java.util.Objects;

public class Athlete implements Cloneable, Comparable<Athlete> {
    protected String name;
    protected int age;
    protected double weight;
    protected double energyLvl;
    protected Equipment equipment;
    protected TrainingZone currentZone;

    protected double x;
    protected double y;
    protected boolean isActive;

    protected Group visualGroup;
    protected ImageView imageView;
    protected Text nameText;
    protected Text equipmentText;
    protected Rectangle textBg;
    protected Rectangle energyBarBg;
    protected Rectangle energyBar;
    protected Rectangle activeBorder;
    protected ImageView zoneEmojiView;

    public Athlete(String name, int age, double weight, double energyLvl, Equipment equipment, double x, double y) {
        this.name = name;
        this.age = age;
        this.weight = weight;
        this.energyLvl = energyLvl;
        this.equipment = equipment;
        this.x = x;
        this.y = y;
        this.isActive = false;
        this.currentZone = null;
        initVisuals();
    }

    protected void initVisuals() {
        visualGroup = new Group();

        activeBorder = new Rectangle(x + 10, y + 50, 140, 210);
        activeBorder.setFill(Color.TRANSPARENT);
        activeBorder.setStrokeWidth(3);

        DropShadow glow = new DropShadow();
        glow.setColor(Color.web("#FF00FF"));
        glow.setRadius(20);
        glow.setSpread(0.5);
        activeBorder.setEffect(glow);
        activeBorder.setVisible(false);

        Duration stepDur = Duration.millis(400);
        StrokeTransition t1 = new StrokeTransition(stepDur, activeBorder, Color.RED, Color.ORANGE);
        StrokeTransition t2 = new StrokeTransition(stepDur, activeBorder, Color.ORANGE, Color.YELLOW);
        StrokeTransition t3 = new StrokeTransition(stepDur, activeBorder, Color.YELLOW, Color.LIME);
        StrokeTransition t4 = new StrokeTransition(stepDur, activeBorder, Color.LIME, Color.CYAN);
        StrokeTransition t5 = new StrokeTransition(stepDur, activeBorder, Color.CYAN, Color.BLUE);
        StrokeTransition t6 = new StrokeTransition(stepDur, activeBorder, Color.BLUE, Color.MAGENTA);
        StrokeTransition t7 = new StrokeTransition(stepDur, activeBorder, Color.MAGENTA, Color.RED);

        SequentialTransition rainbowTransition = new SequentialTransition(t1, t2, t3, t4, t5, t6, t7);
        rainbowTransition.setCycleCount(Timeline.INDEFINITE);
        rainbowTransition.play();

        textBg = new Rectangle(x + 2, y + 2, 156, 40);
        textBg.setFill(Color.WHITE);

        nameText = new Text(x + 5, y + 18, name);
        nameText.setFont(Font.font("Arial", FontWeight.BOLD, 14));
        nameText.setFill(Color.BLACK);

        String eqName = (equipment != null) ? equipment.getName() : "None";
        equipmentText = new Text(x + 5, y + 33, "Equipment: " + eqName);
        equipmentText.setFont(Font.font("Arial", 10));
        equipmentText.setFill(Color.BLACK);

        energyBarBg = new Rectangle(x + 5, y + 40, 150, 8);
        energyBarBg.setFill(Color.WHITE);
        energyBarBg.setStroke(Color.BLACK);
        energyBarBg.setStrokeWidth(1);

        double barWidth = 150 * (energyLvl / 100.0);
        energyBar = new Rectangle(x + 5, y + 40, barWidth, 8);

        zoneEmojiView = new ImageView();
        zoneEmojiView.setX(x + 138);
        zoneEmojiView.setY(y + 4);
        zoneEmojiView.setFitWidth(22);
        zoneEmojiView.setFitHeight(22);
        zoneEmojiView.setPreserveRatio(true);
        zoneEmojiView.setVisible(false);

        imageView = new ImageView(getImagePath());
        imageView.setX(x + 10);
        imageView.setY(y + 50);
        imageView.setFitWidth(140);
        imageView.setFitHeight(210);
        imageView.setPreserveRatio(false);

        visualGroup.getChildren().addAll(imageView, activeBorder, textBg, zoneEmojiView, nameText, equipmentText, energyBarBg, energyBar);

        drawCustomPrimitives();

        visualGroup.setOnMouseClicked(event -> {
            if (event.getButton() == MouseButton.PRIMARY) {
                toggleActive();
            } else if (event.getButton() == MouseButton.SECONDARY) {
                EditDialog.display(this);
            }
            event.consume();
        });
    }

    protected Image getImagePath() {
        return new Image(Objects.requireNonNull(getClass().getResourceAsStream("/gymworld/athlete.png")));
    }

    protected void drawCustomPrimitives() {
        energyBar.setFill(Color.YELLOW);
    }

    public void move(double dx, double dy) {
        this.x += dx;
        this.y += dy;

        activeBorder.setX(this.x + 10);
        activeBorder.setY(this.y + 50);
        textBg.setX(this.x + 2);
        textBg.setY(this.y + 2);
        nameText.setX(this.x + 5);
        nameText.setY(this.y + 18);
        equipmentText.setX(this.x + 5);
        equipmentText.setY(this.y + 33);
        energyBarBg.setX(this.x + 5);
        energyBarBg.setY(this.y + 40);
        energyBar.setX(this.x + 5);
        energyBar.setY(this.y + 40);
        zoneEmojiView.setX(this.x + 138);
        zoneEmojiView.setY(this.y + 4);
        imageView.setX(this.x + 10);
        imageView.setY(this.y + 50);

        visualGroup.toFront();
    }

    public void move(double step) {
        this.move(step, step);
    }

    public void move(double distance, String direction) {
        if ("UP".equals(direction)) this.move(0, -distance);
        else if ("DOWN".equals(direction)) this.move(0, distance);
        else if ("LEFT".equals(direction)) this.move(-distance, 0);
        else if ("RIGHT".equals(direction)) this.move(distance, 0);
    }

    public void toggleActive() {
        isActive = !isActive;
        activeBorder.setVisible(isActive);
        if (isActive) visualGroup.toFront();
    }

    public void setZoneStatus(boolean inZone, Image zoneEmoji) {
        if (inZone) {
            textBg.setFill(Color.LIGHTGREEN);
            if (zoneEmoji != null) {
                zoneEmojiView.setImage(zoneEmoji);
                zoneEmojiView.setVisible(true);
            }
        } else {
            textBg.setFill(Color.WHITE);
            zoneEmojiView.setVisible(false);
        }
    }

    public TrainingZone getCurrentZone() { return currentZone; }
    public void setCurrentZone(TrainingZone zone) { this.currentZone = zone; }
    public double getX() { return x; }
    public double getY() { return y; }
    public void removeFromScene() { GymWorld.rootGroup.getChildren().remove(visualGroup); }
    public Group getVisualGroup() { return visualGroup; }
    public String getName() { return name; }
    public void setName(String name) {
        this.name = name;
        if (nameText != null) nameText.setText(name);
    }
    public int getAge() { return age; }
    public void setAge(int age) { this.age = age; }
    public double getWeight() { return weight; }
    public void setWeight(double weight) { this.weight = weight; }
    public double getEnergyLvl() { return energyLvl; }
    public void setEnergyLvl(double energyLvl) {
        this.energyLvl = energyLvl;
        if (energyBar != null) energyBar.setWidth(150 * (energyLvl / 100.0));
    }
    public Equipment getEquipment() { return equipment; }
    public void setEquipment(Equipment equipment) {
        this.equipment = equipment;
        if (equipmentText != null) equipmentText.setText("Equipment: " + (equipment != null ? equipment.getName() : "None"));
    }
    public String getType() { return this.getClass().getSimpleName(); }
    public void setType(String type) {}
    public boolean isActive() { return isActive; }

    @Override
    public boolean equals(Object obj) {
        if (obj == null || getClass() != obj.getClass()) return false;
        if (this == obj) return true;
        Athlete other = (Athlete) obj;
        return this.name.equals(other.name) && this.age == other.age && this.weight == other.weight;
    }

    @Override
    public int compareTo(Athlete other) {
        return this.name.compareToIgnoreCase(other.name);
    }

    @Override
    public Object clone() throws CloneNotSupportedException {
        Athlete cloned = (Athlete) super.clone();
        if (this.equipment != null) cloned.setEquipment((Equipment) this.equipment.clone());
        cloned.isActive = false;
        cloned.currentZone = null;
        cloned.x += 170;
        cloned.initVisuals();
        return cloned;
    }

    public Rectangle getHitbox() {
        return activeBorder;
    }

    public static java.util.Comparator<Athlete> NameComparator = (a1, a2) -> a1.getName().compareToIgnoreCase(a2.getName());
    public static java.util.Comparator<Athlete> WeightComparator = (a1, a2) -> Double.compare(a1.getWeight(), a2.getWeight());
    public static java.util.Comparator<Athlete> EnergyComparator = (a1, a2) -> Double.compare(a1.getEnergyLvl(), a2.getEnergyLvl());

    public double autoDx = 0;
    public double autoDy = 0;
    public boolean isAutoMoving = false;

    public void reverseAutoX() {
        this.autoDx = -this.autoDx;
    }

    public void reverseAutoY() {
        this.autoDy = -this.autoDy;
    }
}