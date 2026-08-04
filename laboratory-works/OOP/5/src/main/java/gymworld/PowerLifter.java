package gymworld;

import javafx.scene.image.Image;
import javafx.scene.paint.Color;
import java.util.Objects;

public class PowerLifter extends Athlete {

    public PowerLifter(String name, int age, double weight, double energyLvl, Equipment equipment, double x, double y) {
        super(name, age, weight, energyLvl, equipment, x, y);
    }

    @Override
    protected Image getImagePath() {
        return new Image(Objects.requireNonNull(getClass().getResourceAsStream("/gymworld/power_lifter.png")));
    }

    @Override
    protected void drawCustomPrimitives() {
        super.drawCustomPrimitives();
        energyBar.setFill(Color.BLUE);
    }
}