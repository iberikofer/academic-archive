package gymworld;

import javafx.scene.Group;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;
import javafx.scene.text.Text;

public class Minimap {

    private static final double MAP_W = 240;
    private static final double MAP_H = 135;
    private static final double MAP_X = 10;
    private static final double MAP_Y = 10;

    private double scaleX;
    private double scaleY;

    private double viewportW;
    private double viewportH;

    private final Group root;
    private final Rectangle bg;
    private final Rectangle viewportRect;
    private final Group corridorLayer = new Group();
    private final Group zoneLayer    = new Group();
    private final Group athleteLayer = new Group();
    private final java.util.Map<Athlete, Rectangle> dotMap = new java.util.HashMap<>();

    public Minimap(double viewportW, double viewportH) {
        this.viewportW = viewportW;
        this.viewportH = viewportH;
        this.scaleX = MAP_W / GymWorld.WORLD_W;
        this.scaleY = MAP_H / GymWorld.WORLD_H;

        root = new Group();

        bg = new Rectangle(MAP_X, MAP_Y, MAP_W, MAP_H);
        bg.setFill(Color.color(0.1, 0.1, 0.1, 0.75));
        bg.setStroke(Color.GRAY);
        bg.setStrokeWidth(1);

        viewportRect = new Rectangle(MAP_X, MAP_Y,
                viewportW * scaleX,
                viewportH * scaleY);
        viewportRect.setFill(Color.color(1, 1, 1, 0.25));
        viewportRect.setStroke(Color.WHITE);
        viewportRect.setStrokeWidth(1);

        Text label = new Text(MAP_X + 3, MAP_Y + MAP_H + 12, "Minimap  [click to move camera]");
        label.setFont(Font.font("Arial", FontWeight.NORMAL, 9));
        label.setFill(Color.BLACK);

        root.getChildren().addAll(bg, corridorLayer, zoneLayer, athleteLayer, viewportRect, label);

        root.setOnMouseClicked(event -> {
            double relX = event.getX() - MAP_X;
            double relY = event.getY() - MAP_Y;

            if (relX < 0 || relY < 0 || relX > MAP_W || relY > MAP_H) return;

            double worldX = relX / scaleX;
            double worldY = relY / scaleY;

            double newCamX = worldX - viewportW / 2.0;
            double newCamY = worldY - viewportH / 2.0;

            newCamX = Math.max(0, Math.min(GymWorld.WORLD_W - viewportW, newCamX));
            newCamY = Math.max(0, Math.min(GymWorld.WORLD_H - viewportH, newCamY));

            GymWorld.camX = newCamX;
            GymWorld.camY = newCamY;
            GymWorld.applyCamera();
        });
    }

    public void resize(double newViewportW, double newViewportH) {
        this.viewportW = newViewportW;
        this.viewportH = newViewportH;
        this.scaleX = MAP_W / GymWorld.WORLD_W;
        this.scaleY = MAP_H / GymWorld.WORLD_H;
        viewportRect.setWidth(newViewportW * scaleX);
        viewportRect.setHeight(newViewportH * scaleY);
        zoneLayer.getChildren().clear();
        corridorLayer.getChildren().clear();
    }

    public void update(double cameraTransX, double cameraTransY) {
        if (corridorLayer.getChildren().isEmpty() && !GymWorld.corridors.isEmpty()) {
            for (Rectangle cor : GymWorld.corridors) {
                Rectangle r = new Rectangle(
                        MAP_X + cor.getX() * scaleX,
                        MAP_Y + cor.getY() * scaleY,
                        cor.getWidth() * scaleX,
                        cor.getHeight() * scaleY);
                r.setFill(cor.getFill());
                corridorLayer.getChildren().add(r);
            }
        }

        if (zoneLayer.getChildren().size() != GymWorld.zones.size()) {
            zoneLayer.getChildren().clear();
            for (TrainingZone zone : GymWorld.zones) {
                Rectangle r = new Rectangle(
                        MAP_X + zone.getX() * scaleX,
                        MAP_Y + zone.getY() * scaleY,
                        zone.getWidth()  * scaleX,
                        zone.getHeight() * scaleY);
                if (zone.getName().equalsIgnoreCase("Front Desk")) {
                    r.setFill(Color.color(0.9, 0.2, 0.2, 0.5));
                    r.setStroke(Color.RED);
                } else if (zone.getName().equalsIgnoreCase("Cardio Area")) {
                    r.setFill(Color.color(0.9, 0.9, 0.2, 0.5));
                    r.setStroke(Color.YELLOW);
                } else {
                    r.setFill(Color.color(0.2, 0.5, 0.9, 0.5));
                    r.setStroke(Color.CORNFLOWERBLUE);
                }
                r.setStrokeWidth(0.5);
                zoneLayer.getChildren().add(r);
            }
        }

        dotMap.keySet().removeIf(a -> {
            if (!GymWorld.athletes.contains(a)) {
                athleteLayer.getChildren().remove(dotMap.get(a));
                return true;
            }
            return false;
        });

        for (Athlete a : GymWorld.athletes) {
            Rectangle dot = dotMap.get(a);
            if (dot == null) {
                dot = new Rectangle(0, 0, 4, 4);
                if      (a instanceof GymBoss)     dot.setFill(Color.RED);
                else if (a instanceof PowerLifter) dot.setFill(Color.BLUE);
                else                               dot.setFill(Color.YELLOW);
                dot.setStroke(Color.BLACK);
                dot.setStrokeWidth(0.5);
                dotMap.put(a, dot);
                athleteLayer.getChildren().add(dot);
            }
            dot.setX(MAP_X + a.getX() * scaleX);
            dot.setY(MAP_Y + a.getY() * scaleY);
        }

        viewportRect.setX(MAP_X + (-cameraTransX) * scaleX);
        viewportRect.setY(MAP_Y + (-cameraTransY) * scaleY);
    }

    public void reset() {
        zoneLayer.getChildren().clear();
        corridorLayer.getChildren().clear();
        athleteLayer.getChildren().clear();
        dotMap.clear();
    }

    public Group getRoot() { return root; }
}