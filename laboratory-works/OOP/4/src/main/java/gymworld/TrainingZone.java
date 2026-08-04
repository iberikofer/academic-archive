package gymworld;

import javafx.scene.Group;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.scene.text.Text;

import java.util.Arrays;
import java.util.Objects;

public class TrainingZone {
    private String name;
    private double x;
    private double y;
    private double width;
    private double height;
    private Athlete[] zoneAthletes;

    private Group visualGroup;
    private ImageView background;
    private Text nameText;
    private Text countText;
    private ImageView iconImage;
    private Rectangle border;

    public TrainingZone(String name, double x, double y, double width, double height, String imagePath, String emojiPath) {
        this.name = name;
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        this.zoneAthletes = new Athlete[0];
        initVisuals(imagePath, emojiPath);
    }

    private void initVisuals(String imagePath, String emojiPath) {
        visualGroup = new Group();

        Image bgImage = new Image(Objects.requireNonNull(getClass().getResourceAsStream(imagePath)));
        background = new ImageView(bgImage);
        background.setX(x);
        background.setY(y);
        background.setFitWidth(width);
        background.setFitHeight(height);

        border = new Rectangle(x, y, width, height);
        border.setFill(Color.TRANSPARENT);
        border.setStroke(Color.TRANSPARENT);

        Image emojiImg = new Image(Objects.requireNonNull(getClass().getResourceAsStream(emojiPath)));
        iconImage = new ImageView(emojiImg);
        iconImage.setX(x + 5);
        iconImage.setY(y + height + 5);
        iconImage.setFitWidth(24);
        iconImage.setFitHeight(24);
        iconImage.setPreserveRatio(true);

        nameText = new Text(x + 35, y + height + 23, name + ":");
        nameText.setFont(Font.font("Arial", FontWeight.BOLD, 16));

        countText = new Text(x + width - 40, y + height + 23, "0");
        countText.setFont(Font.font("Arial", FontWeight.BOLD, 18));
        countText.setFill(Color.RED);

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
            if (zoneAthletes[i].equals(athlete)) {
                index = i;
                break;
            }
        }
        if (index != -1) {
            Athlete[] newArray = new Athlete[zoneAthletes.length - 1];
            for (int i = 0, j = 0; i < zoneAthletes.length; i++) {
                if (i != index) {
                    newArray[j++] = zoneAthletes[i];
                }
            }
            zoneAthletes = newArray;
            updateCount();
        }
    }

    private void updateCount() {
        countText.setText(String.valueOf(zoneAthletes.length));
        if (zoneAthletes.length == 0) {
            countText.setFill(Color.RED);
        } else {
            countText.setFill(Color.BLACK);
        }
    }

    public Group getVisualGroup() {
        return visualGroup;
    }

    public String getName() {
        return name;
    }

    public Image getEmojiImage() {
        return iconImage.getImage();
    }
}