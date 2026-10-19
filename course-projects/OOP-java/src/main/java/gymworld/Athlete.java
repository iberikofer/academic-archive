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

import java.io.Serializable;
import java.util.Comparator;
import java.util.Objects;

public class Athlete implements Cloneable, Comparable<Athlete>, Serializable {

    private static final long serialVersionUID = 1L;

    protected static final double IMG_W  = 84;
    protected static final double IMG_H  = 136.5;
    protected static final double BAR_W  = 78 * 1.65;
    protected static final double UI_H   = 45;
    protected static final double TOTAL_H = UI_H + IMG_H;

    protected String name;
    protected int age;
    protected double weight;
    protected double energyLvl;
    protected Equipment equipment;
    protected TrainingZone currentZone;

    protected double x;
    protected double y;
    protected boolean isActive;

    protected transient Group visualGroup;
    protected transient ImageView imageView;
    protected transient Text nameText;
    protected transient Text equipmentText;
    protected transient Rectangle textBg;
    protected transient Rectangle energyBarBg;
    protected transient Rectangle energyBar;
    protected transient Rectangle activeBorder;
    protected transient ImageView zoneEmojiView;

    public double autoDx = 0;
    public double autoDy = 0;
    public boolean isAutoMoving = false;

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

    public void initVisuals() {
        visualGroup = new Group();

        activeBorder = new Rectangle(x, y + UI_H, IMG_W + 4, IMG_H);
        activeBorder.setFill(Color.TRANSPARENT);
        activeBorder.setStrokeWidth(2);

        DropShadow glow = new DropShadow();
        glow.setColor(Color.web("#FF00FF"));
        glow.setRadius(12);
        glow.setSpread(0.5);
        activeBorder.setEffect(glow);
        activeBorder.setVisible(false);

        Duration stepDur = Duration.millis(400);
        StrokeTransition t1 = new StrokeTransition(stepDur, activeBorder, Color.RED,     Color.ORANGE);
        StrokeTransition t2 = new StrokeTransition(stepDur, activeBorder, Color.ORANGE,  Color.YELLOW);
        StrokeTransition t3 = new StrokeTransition(stepDur, activeBorder, Color.YELLOW,  Color.LIME);
        StrokeTransition t4 = new StrokeTransition(stepDur, activeBorder, Color.LIME,    Color.CYAN);
        StrokeTransition t5 = new StrokeTransition(stepDur, activeBorder, Color.CYAN,    Color.BLUE);
        StrokeTransition t6 = new StrokeTransition(stepDur, activeBorder, Color.BLUE,    Color.MAGENTA);
        StrokeTransition t7 = new StrokeTransition(stepDur, activeBorder, Color.MAGENTA, Color.RED);
        SequentialTransition rainbow = new SequentialTransition(t1, t2, t3, t4, t5, t6, t7);
        rainbow.setCycleCount(Timeline.INDEFINITE);
        rainbow.play();

        double infoW = (IMG_W + 4) * 1.65;
        double infoX = x + 2 + IMG_W / 2.0 - infoW / 2.0;

        textBg = new Rectangle(infoX, y, infoW, UI_H - 2);
        textBg.setFill(Color.WHITE);

        nameText = new Text(infoX + 6, y + 16, name);
        nameText.setFont(Font.font("Arial", FontWeight.BOLD, 13));
        nameText.setFill(Color.BLACK);

        String eqName = (equipment != null) ? equipment.getName() : "None";
        equipmentText = new Text(infoX + 6, y + 29, "Eq: " + eqName);
        equipmentText.setFont(Font.font("Arial", 10));
        equipmentText.setFill(Color.BLACK);

        double barX = infoX + (infoW - BAR_W) / 2.0;
        energyBarBg = new Rectangle(barX, y + UI_H - 9, BAR_W, 7);
        energyBarBg.setFill(Color.WHITE);
        energyBarBg.setStroke(Color.BLACK);
        energyBarBg.setStrokeWidth(0.5);

        double barWidth = BAR_W * (energyLvl / 100.0);
        energyBar = new Rectangle(barX, y + UI_H - 9, barWidth, 7);

        zoneEmojiView = new ImageView();
        zoneEmojiView.setX(infoX + infoW - 25);
        zoneEmojiView.setY(y + 3);
        zoneEmojiView.setFitWidth(21);
        zoneEmojiView.setFitHeight(21);
        zoneEmojiView.setPreserveRatio(true);
        zoneEmojiView.setVisible(false);

        imageView = new ImageView(getImagePath());
        imageView.setX(x + 2);
        imageView.setY(y + UI_H);
        imageView.setFitWidth(IMG_W);
        imageView.setFitHeight(IMG_H);
        imageView.setPreserveRatio(false);

        visualGroup.getChildren().addAll(
                imageView, activeBorder, textBg, zoneEmojiView,
                nameText, equipmentText, energyBarBg, energyBar);

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

        double infoW = (IMG_W + 4) * 1.65;
        double infoX = this.x + 2 + IMG_W / 2.0 - infoW / 2.0;
        double barX = infoX + (infoW - BAR_W) / 2.0;

        activeBorder.setX(this.x);
        activeBorder.setY(this.y + UI_H);
        textBg.setX(infoX);
        textBg.setY(this.y);
        nameText.setX(infoX + 6);
        nameText.setY(this.y + 16);
        equipmentText.setX(infoX + 6);
        equipmentText.setY(this.y + 29);
        energyBarBg.setX(barX);
        energyBarBg.setY(this.y + UI_H - 9);
        energyBar.setX(barX);
        energyBar.setY(this.y + UI_H - 9);
        zoneEmojiView.setX(infoX + infoW - 25);
        zoneEmojiView.setY(this.y + 3);
        imageView.setX(this.x + 2);
        imageView.setY(this.y + UI_H);

        visualGroup.toFront();
    }

    public void move(double step) { move(step, step); }

    public void move(double distance, String direction) {
        if      ("UP".equals(direction))    move(0, -distance);
        else if ("DOWN".equals(direction))  move(0,  distance);
        else if ("LEFT".equals(direction))  move(-distance, 0);
        else if ("RIGHT".equals(direction)) move( distance, 0);
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

    public TrainingZone getCurrentZone()          { return currentZone; }
    public void setCurrentZone(TrainingZone zone) { this.currentZone = zone; }
    public double getX()                          { return x; }
    public double getY()                          { return y; }
    public void removeFromScene()                 { GymWorld.rootGroup.getChildren().remove(visualGroup); }
    public Group getVisualGroup()                 { return visualGroup; }
    public String getName()                       { return name; }
    public void setName(String name) {
        this.name = name;
        if (nameText != null) nameText.setText(name);
    }
    public int getAge()                           { return age; }
    public void setAge(int age)                   { this.age = age; }
    public double getWeight()                     { return weight; }
    public void setWeight(double weight)          { this.weight = weight; }
    public double getEnergyLvl()                  { return energyLvl; }
    public void setEnergyLvl(double energyLvl) {
        this.energyLvl = energyLvl;
        if (energyBar != null) energyBar.setWidth(BAR_W * (energyLvl / 100.0));
    }
    public Equipment getEquipment()               { return equipment; }
    public void setEquipment(Equipment equipment) {
        this.equipment = equipment;
        if (equipmentText != null)
            equipmentText.setText("Eq: " + (equipment != null ? equipment.getName() : "None"));
    }
    public String getType()                       { return this.getClass().getSimpleName(); }
    public void setType(String type)              {}
    public boolean isActive()                     { return isActive; }

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
        cloned.x += IMG_W + 10;
        cloned.initVisuals();
        return cloned;
    }

    public Rectangle getHitbox() { return activeBorder; }

    public static final Comparator<Athlete> NameComparator   = (a1, a2) -> a1.getName().compareToIgnoreCase(a2.getName());
    public static final Comparator<Athlete> WeightComparator = (a1, a2) -> Double.compare(a1.getWeight(), a2.getWeight());
    public static final Comparator<Athlete> EnergyComparator = (a1, a2) -> Double.compare(a1.getEnergyLvl(), a2.getEnergyLvl());

    public void reverseAutoX() { this.autoDx = -this.autoDx; }
    public void reverseAutoY() { this.autoDy = -this.autoDy; }
}