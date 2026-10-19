package gymworld;

import javafx.scene.Group;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.scene.text.Text;

import java.io.Serializable;
import java.util.Arrays;
import java.util.Objects;

public class TrainingZone implements Serializable {

    private static final long serialVersionUID = 1L;

    private String name;
    private double x;
    private double y;
    private double width;
    private double height;
    private Athlete[] zoneAthletes;

    private String imagePath;
    private String emojiPath;

    private transient Group visualGroup;
    private transient ImageView background;
    private transient Text nameText;
    private transient Text countText;
    private transient ImageView iconImage;
    private transient Rectangle border;

    public TrainingZone(String name, double x, double y, double width, double height,
                        String imagePath, String emojiPath) {
        this.name      = name;
        this.x         = x;
        this.y         = y;
        this.width     = width;
        this.height    = height;
        this.imagePath = imagePath;
        this.emojiPath = emojiPath;
        this.zoneAthletes = new Athlete[0];
        initVisuals(imagePath, emojiPath);
    }

    public void initVisuals() {
        initVisuals(this.imagePath, this.emojiPath);
    }

    private void initVisuals(String imgPath, String emjPath) {
        visualGroup = new Group();

        Image bgImage = new Image(Objects.requireNonNull(getClass().getResourceAsStream(imgPath)));
        background = new ImageView(bgImage);
        background.setX(x);
        background.setY(y);
        background.setFitWidth(width);
        background.setFitHeight(height);

        border = new Rectangle(x, y, width, height);
        border.setFill(Color.TRANSPARENT);
        border.setStroke(Color.TRANSPARENT);

        Image emojiImg = new Image(Objects.requireNonNull(getClass().getResourceAsStream(emjPath)));
        iconImage = new ImageView(emojiImg);
        iconImage.setX(x + 5);
        iconImage.setY(y + height + 5);
        iconImage.setFitWidth(24);
        iconImage.setFitHeight(24);
        iconImage.setPreserveRatio(true);

        nameText = new Text(x + 35, y + height + 23, name + ":");
        nameText.setFont(Font.font("Arial", FontWeight.BOLD, 16));

        countText = new Text(x + width - 40, y + height + 23, String.valueOf(zoneAthletes.length));
        countText.setFont(Font.font("Arial", FontWeight.BOLD, 18));
        countText.setFill(zoneAthletes.length == 0 ? Color.RED : Color.BLACK);

        visualGroup.getChildren().addAll(background, border, iconImage, nameText, countText);
    }

    public void addAthlete(Athlete athlete) {
        zoneAthletes = Arrays.copyOf(zoneAthletes, zoneAthletes.length + 1);
        zoneAthletes[zoneAthletes.length - 1] = athlete;
        updateCount();
    }

    public void removeAthlete(Athlete athlete) {
        int index = -1;
        for (int i = 0; i < zoneAthletes.length; i++) {
            if (zoneAthletes[i].equals(athlete)) { index = i; break; }
        }
        if (index != -1) {
            Athlete[] newArray = new Athlete[zoneAthletes.length - 1];
            for (int i = 0, j = 0; i < zoneAthletes.length; i++) {
                if (i != index) newArray[j++] = zoneAthletes[i];
            }
            zoneAthletes = newArray;
            updateCount();
        }
    }

    private void updateCount() {
        if (countText == null) return;
        countText.setText(String.valueOf(zoneAthletes.length));
        countText.setFill(zoneAthletes.length == 0 ? Color.RED : Color.BLACK);
    }

    public Group getVisualGroup()   { return visualGroup; }
    public String getName()         { return name; }
    public Image getEmojiImage()    { return iconImage.getImage(); }
    public Rectangle getHitbox()    { return border; }
    public double getX()            { return x; }
    public double getY()            { return y; }
    public double getWidth()        { return width; }
    public double getHeight()       { return height; }
}