package gymworld;

import javafx.animation.AnimationTimer;
import javafx.application.Application;
import javafx.scene.Group;
import javafx.scene.Scene;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyCombination;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.scene.text.Font;
import javafx.scene.text.Text;
import javafx.stage.Screen;
import javafx.stage.Stage;
import javafx.stage.StageStyle;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class GymWorld extends Application {

    public static double WORLD_W = 3840;
    public static double WORLD_H = 2160;

    public static double SCREEN_W = 960;
    public static double SCREEN_H = 540;

    public static Stage  mainStage;
    public static Group  rootGroup;
    public static List<Athlete>      athletes = new ArrayList<>();
    public static List<TrainingZone> zones    = new ArrayList<>();
    public static final List<Rectangle> corridors = new ArrayList<>();

    private final Set<KeyCode> activeKeys = new HashSet<>();
    public  static boolean solidMode = false;

    public  static double camX = 0;
    public  static double camY = 0;
    private static final double CAM_SPEED = 6;

    private static double FD_W = 900;
    private static double FD_H = 500;
    private static double FD_X;
    private static double FD_Y;

    private static double CA_W = 800;
    private static double CA_H = 600;
    private static double CA_X;
    private static double CA_Y;

    private static double WA_W = 800;
    private static double WA_H = 600;
    private static double WA_X;
    private static double WA_Y;

    private static double COR_W = 260;

    public static Minimap minimap;
    private static Rectangle worldClip;
    private Rectangle hudBg;
    private Text hudText;

    @Override
    public void start(Stage stage) {
        mainStage = stage;

        javafx.geometry.Rectangle2D screenBounds = Screen.getPrimary().getBounds();
        SCREEN_W = screenBounds.getWidth();
        SCREEN_H = screenBounds.getHeight();
        WORLD_W = SCREEN_W * 2.0;
        WORLD_H = SCREEN_H * 2.0;

        FD_X = (WORLD_W - FD_W) / 2.0;
        FD_Y = 80;

        CA_X = 200;
        CA_Y = WORLD_H - CA_H - 80;

        WA_X = WORLD_W - WA_W - 200;
        WA_Y = WORLD_H - WA_H - 80;

        rootGroup = new Group();
        Group sceneRoot = new Group(rootGroup);

        Scene scene = new Scene(sceneRoot, SCREEN_W, SCREEN_H);
        scene.setFill(Color.web("#E5E5E5"));

        worldClip = new Rectangle(0, 0, SCREEN_W, SCREEN_H);
        rootGroup.setClip(worldClip);

        TrainingZone frontDesk   = new TrainingZone("Front Desk",   FD_X, FD_Y, FD_W, FD_H,
                "/gymworld/front_desk.png",   "/gymworld/wave.png");
        TrainingZone cardioArea  = new TrainingZone("Cardio Area",  CA_X, CA_Y, CA_W, CA_H,
                "/gymworld/cardio_area.png",  "/gymworld/lightning.png");
        TrainingZone weightsArea = new TrainingZone("Weights Area", WA_X, WA_Y, WA_W, WA_H,
                "/gymworld/weights_area.png", "/gymworld/bicep.png");

        zones.add(frontDesk);
        zones.add(cardioArea);
        zones.add(weightsArea);

        Rectangle lcHBase = new Rectangle(CA_X, FD_Y + FD_H / 2.0 - COR_W / 2.0, FD_X - CA_X, COR_W);
        Rectangle lcHInner = new Rectangle(CA_X + 10, FD_Y + FD_H / 2.0 - COR_W / 2.0 + 10, FD_X - CA_X, COR_W - 20);
        Rectangle lcVBase = new Rectangle(CA_X, FD_Y + FD_H / 2.0 - COR_W / 2.0, COR_W, CA_Y - (FD_Y + FD_H / 2.0 - COR_W / 2.0));
        Rectangle lcVInner = new Rectangle(CA_X + 10, FD_Y + FD_H / 2.0 - COR_W / 2.0 + 10, COR_W - 20, CA_Y - (FD_Y + FD_H / 2.0 - COR_W / 2.0) - 10);

        Rectangle rcHBase = new Rectangle(FD_X + FD_W, FD_Y + FD_H / 2.0 - COR_W / 2.0, (WA_X + WA_W) - (FD_X + FD_W), COR_W);
        Rectangle rcHInner = new Rectangle(FD_X + FD_W, FD_Y + FD_H / 2.0 - COR_W / 2.0 + 10, (WA_X + WA_W - 10) - (FD_X + FD_W), COR_W - 20);
        Rectangle rcVBase = new Rectangle(WA_X + WA_W - COR_W, FD_Y + FD_H / 2.0 - COR_W / 2.0, COR_W, WA_Y - (FD_Y + FD_H / 2.0 - COR_W / 2.0));
        Rectangle rcVInner = new Rectangle(WA_X + WA_W - COR_W + 10, FD_Y + FD_H / 2.0 - COR_W / 2.0 + 10, COR_W - 20, WA_Y - (FD_Y + FD_H / 2.0 - COR_W / 2.0) - 10);

        Rectangle ccVBase = new Rectangle(FD_X + FD_W / 2.0 - COR_W / 2.0, FD_Y + FD_H, COR_W, (CA_Y + CA_H / 2.0 + COR_W / 2.0) - (FD_Y + FD_H));
        Rectangle ccVInner = new Rectangle(FD_X + FD_W / 2.0 - COR_W / 2.0 + 10, FD_Y + FD_H, COR_W - 20, (CA_Y + CA_H / 2.0 + COR_W / 2.0 - 10) - (FD_Y + FD_H));
        Rectangle ccLBase = new Rectangle(CA_X + CA_W, CA_Y + CA_H / 2.0 - COR_W / 2.0, (FD_X + FD_W / 2.0 - COR_W / 2.0) - (CA_X + CA_W), COR_W);
        Rectangle ccLInner = new Rectangle(CA_X + CA_W, CA_Y + CA_H / 2.0 - COR_W / 2.0 + 10, (FD_X + FD_W / 2.0 - COR_W / 2.0 + 10) - (CA_X + CA_W), COR_W - 20);
        Rectangle ccRBase = new Rectangle(FD_X + FD_W / 2.0 + COR_W / 2.0, WA_Y + WA_H / 2.0 - COR_W / 2.0, WA_X - (FD_X + FD_W / 2.0 + COR_W / 2.0), COR_W);
        Rectangle ccRInner = new Rectangle(FD_X + FD_W / 2.0 + COR_W / 2.0 - 10, WA_Y + WA_H / 2.0 - COR_W / 2.0 + 10, WA_X - (FD_X + FD_W / 2.0 + COR_W / 2.0 - 10), COR_W - 20);

        lcHBase.setFill(Color.web("#A0A0A0"));
        lcHInner.setFill(Color.web("#606060"));
        lcVBase.setFill(Color.web("#A0A0A0"));
        lcVInner.setFill(Color.web("#606060"));

        rcHBase.setFill(Color.web("#A0A0A0"));
        rcHInner.setFill(Color.web("#606060"));
        rcVBase.setFill(Color.web("#A0A0A0"));
        rcVInner.setFill(Color.web("#606060"));

        ccVBase.setFill(Color.web("#A0A0A0"));
        ccVInner.setFill(Color.web("#606060"));
        ccLBase.setFill(Color.web("#A0A0A0"));
        ccLInner.setFill(Color.web("#606060"));
        ccRBase.setFill(Color.web("#A0A0A0"));
        ccRInner.setFill(Color.web("#606060"));

        corridors.clear();
        corridors.addAll(List.of(
                lcHBase, lcVBase,
                rcHBase, rcVBase,
                ccVBase, ccLBase, ccRBase,
                lcHInner, lcVInner,
                rcHInner, rcVInner,
                ccVInner, ccLInner, ccRInner
        ));

        rootGroup.getChildren().addAll(
                lcHBase, lcVBase,
                rcHBase, rcVBase,
                ccVBase, ccLBase, ccRBase,
                lcHInner, lcVInner,
                rcHInner, rcVInner,
                ccVInner, ccLInner, ccRInner,
                frontDesk.getVisualGroup(),
                cardioArea.getVisualGroup(),
                weightsArea.getVisualGroup()
        );

        Athlete testAthlete = new Athlete("Yarik",        20,  80.0, 100.0,
                new Equipment("Exercise band", 5,  "kg"), CA_X + 60, CA_Y + 60);
        Athlete powerLifter = new PowerLifter("Big Vasya", 35, 120.0, 100.0,
                new Equipment("Weight belt",  10,  "kg"), WA_X + 60, WA_Y + 60);
        Athlete gymBoss     = new GymBoss("Boss Ruslan",  45,  95.0, 100.0,
                new Equipment("Water bottle",  1,  "kg"), FD_X + 60, FD_Y + 60);

        athletes.add(testAthlete);
        athletes.add(powerLifter);
        athletes.add(gymBoss);
        for (Athlete a : athletes) assignToZoneInitially(a);

        rootGroup.getChildren().addAll(
                testAthlete.getVisualGroup(),
                powerLifter.getVisualGroup(),
                gymBoss.getVisualGroup()
        );

        minimap = new Minimap(SCREEN_W, SCREEN_H);
        sceneRoot.getChildren().add(minimap.getRoot());

        double hudH = 20;
        hudBg = new Rectangle(0, SCREEN_H - hudH, SCREEN_W, hudH);
        hudBg.setFill(Color.web("#12161A", 0.90));
        hudBg.setStroke(Color.web("#FF5500", 0.8));
        hudBg.setStrokeWidth(1);
        hudBg.setMouseTransparent(true);
        hudText = new Text(8, SCREEN_H - 5,
                "Arrows/WASD = camera/move  |  INSERT = add  |  F = find  |  G = queries  |  P = auto  |  " +
                        "B = solid  |  Ctrl+C = clone  |  DEL = delete  |  Ctrl+S = save  |  Ctrl+L = load  |  ESC = deselect");
        hudText.setFont(Font.font("Segoe UI", javafx.scene.text.FontWeight.BOLD, 10));
        hudText.setFill(Color.web("#E0E4E8"));
        hudText.setMouseTransparent(true);
        sceneRoot.getChildren().addAll(hudBg, hudText);

        scene.setOnKeyPressed(event -> {
            activeKeys.add(event.getCode());

            if (event.getCode() == KeyCode.INSERT) {
                activeKeys.remove(KeyCode.INSERT);
                Athlete newAthlete = InsertDialog.display();
                if (newAthlete != null) {
                    athletes.add(newAthlete);
                    assignToZoneInitially(newAthlete);
                    rootGroup.getChildren().add(newAthlete.getVisualGroup());
                    newAthlete.getVisualGroup().toFront();
                }
            }
            if (event.getCode() == KeyCode.F) {
                activeKeys.remove(KeyCode.F);
                SearchDialog.display();
            }
            if (event.getCode() == KeyCode.G) {
                activeKeys.remove(KeyCode.G);
                FilterDialog.display();
            }
            if (event.getCode() == KeyCode.P) {
                activeKeys.remove(KeyCode.P);
                athletes.stream().filter(a -> !a.isActive()).forEach(a -> {
                    a.isAutoMoving = !a.isAutoMoving;
                    if (a.isAutoMoving) {
                        a.autoDx = (Math.random() > 0.5 ? 2.5 : -2.5);
                        a.autoDy = (Math.random() > 0.5 ? 2.5 : -2.5);
                    }
                });
            }
            if (event.getCode() == KeyCode.B) {
                activeKeys.remove(KeyCode.B);
                solidMode = !solidMode;
            }
            if (event.isControlDown() && event.getCode() == KeyCode.C) {
                List<Athlete> clones = new ArrayList<>();
                athletes.stream().filter(Athlete::isActive).forEach(a -> {
                    try {
                        Athlete cl = (Athlete) a.clone();
                        clones.add(cl);
                        rootGroup.getChildren().add(cl.getVisualGroup());
                        cl.getVisualGroup().toFront();
                    } catch (CloneNotSupportedException ex) { ex.printStackTrace(); }
                });
                athletes.addAll(clones);
            }
            if (event.isControlDown() && event.getCode() == KeyCode.S) {
                activeKeys.remove(KeyCode.S);
                SerializationManager.save();
            }
            if (event.isControlDown() && event.getCode() == KeyCode.L) {
                activeKeys.remove(KeyCode.L);
                SerializationManager.load();
            }
            if (event.getCode() == KeyCode.ESCAPE) {
                athletes.stream().filter(Athlete::isActive).forEach(Athlete::toggleActive);
            }
            if (event.getCode() == KeyCode.DELETE) {
                List<Athlete> toRemove = athletes.stream().filter(Athlete::isActive)
                        .collect(java.util.stream.Collectors.toList());
                toRemove.forEach(a -> {
                    if (a.getCurrentZone() != null) a.getCurrentZone().removeAthlete(a);
                    a.removeFromScene();
                });
                athletes.removeAll(toRemove);
            }
        });

        scene.setOnKeyReleased(event -> activeKeys.remove(event.getCode()));

        AnimationTimer timer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                double dcx = 0, dcy = 0;
                if (activeKeys.contains(KeyCode.LEFT)  || activeKeys.contains(KeyCode.A)) dcx = -CAM_SPEED;
                if (activeKeys.contains(KeyCode.RIGHT) || activeKeys.contains(KeyCode.D)) dcx =  CAM_SPEED;
                if (activeKeys.contains(KeyCode.UP)    || activeKeys.contains(KeyCode.W)) dcy = -CAM_SPEED;
                if (activeKeys.contains(KeyCode.DOWN)  || activeKeys.contains(KeyCode.S)) dcy =  CAM_SPEED;

                boolean anyActive = athletes.stream().anyMatch(Athlete::isActive);

                if (!anyActive && (dcx != 0 || dcy != 0)) {
                    camX = Math.max(0, Math.min(WORLD_W - SCREEN_W, camX + dcx));
                    camY = Math.max(0, Math.min(WORLD_H - SCREEN_H, camY + dcy));
                    applyCamera();
                }

                for (Athlete a : athletes) {
                    if (a.isActive() && (dcx != 0 || dcy != 0)) {
                        double nextX = a.getX() + dcx;
                        double nextY = a.getY() + dcy;
                        if (canMoveTo(a, nextX, nextY)) {
                            a.move(dcx, dcy);
                            handleCollisions(a);
                            updateZone(a);
                            adjustCameraToAthlete(a);
                        }
                    } else if (!a.isActive() && a.isAutoMoving) {
                        autoMove(a);
                    }
                }

                minimap.update(rootGroup.getTranslateX(), rootGroup.getTranslateY());
            }
        };
        timer.start();

        stage.initStyle(StageStyle.UNDECORATED);
        stage.setTitle("GymWorld");
        stage.setScene(scene);
        stage.setFullScreen(true);
        stage.setFullScreenExitKeyCombination(KeyCombination.NO_MATCH);
        stage.show();

        scene.widthProperty().addListener((obs, oldW, newW) -> onScreenResize(newW.doubleValue(), scene.getHeight()));
        scene.heightProperty().addListener((obs, oldH, newH) -> onScreenResize(scene.getWidth(), newH.doubleValue()));
    }

    private void onScreenResize(double w, double h) {
        SCREEN_W = w;
        SCREEN_H = h;
        worldClip.setWidth(w);
        worldClip.setHeight(h);
        if (hudBg  != null) { hudBg.setY(h - 20); hudBg.setWidth(w); }
        if (hudText != null) { hudText.setY(h - 5); }
        if (minimap != null) minimap.resize(w, h);
        camX = Math.min(camX, WORLD_W - w);
        camY = Math.min(camY, WORLD_H - h);
        applyCamera();
    }

    public static void applyCamera() {
        rootGroup.setTranslateX(-camX);
        rootGroup.setTranslateY(-camY);
        worldClip.setX(camX);
        worldClip.setY(camY);
    }

    private void adjustCameraToAthlete(Athlete a) {
        double margin = 120.0;
        double screenX = a.getX() - camX;
        double screenY = a.getY() - camY;
        double targetCamX = camX;
        double targetCamY = camY;

        if (screenX < margin) {
            targetCamX = a.getX() - margin;
        } else if (screenX > SCREEN_W - Athlete.IMG_W - margin) {
            targetCamX = a.getX() - (SCREEN_W - Athlete.IMG_W - margin);
        }

        if (screenY < margin) {
            targetCamY = a.getY() - margin;
        } else if (screenY > SCREEN_H - Athlete.TOTAL_H - margin) {
            targetCamY = a.getY() - (SCREEN_H - Athlete.TOTAL_H - margin);
        }

        targetCamX = Math.max(0, Math.min(WORLD_W - SCREEN_W, targetCamX));
        targetCamY = Math.max(0, Math.min(WORLD_H - SCREEN_H, targetCamY));

        if (targetCamX != camX || targetCamY != camY) {
            camX = targetCamX;
            camY = targetCamY;
            applyCamera();
        }
    }

    private void autoMove(Athlete a) {
        if (a.autoDx == 0 && a.autoDy == 0) {
            a.autoDx = (Math.random() > 0.5 ? 2.5 : -2.5);
            a.autoDy = (Math.random() > 0.5 ? 2.5 : -2.5);
        }

        double nextX = a.getX() + a.autoDx;
        double nextY = a.getY() + a.autoDy;

        boolean validNext = canMoveTo(a, nextX, nextY);
        if (!validNext) {
            boolean validRevX = canMoveTo(a, a.getX() - a.autoDx, nextY);
            boolean validRevY = canMoveTo(a, nextX, a.getY() - a.autoDy);

            if (validRevX && !validRevY) {
                a.reverseAutoX();
            } else if (validRevY && !validRevX) {
                a.reverseAutoY();
            } else {
                a.reverseAutoX();
                a.reverseAutoY();
            }
        }

        a.move(a.autoDx, a.autoDy);

        for (Athlete other : athletes) {
            if (other instanceof GymBoss) {
                if (a != other && a.getHitbox().getBoundsInParent().intersects(other.getHitbox().getBoundsInParent())) {
                    double e = a.getEnergyLvl();
                    if (e > 0.5) a.setEnergyLvl(e - 0.5);
                }
            }
        }
        applyZone(a, detectZone(a));
    }

    private void handleCollisions(Athlete a) {
        for (Athlete other : athletes) {
            if (other instanceof GymBoss) {
                if (a != other && a.getHitbox().getBoundsInParent().intersects(other.getHitbox().getBoundsInParent())) {
                    double e = a.getEnergyLvl();
                    if (e > 0.5) a.setEnergyLvl(e - 0.5);
                }
            }
        }
    }

    private TrainingZone detectZone(Athlete a) {
        return zones.stream()
                .filter(z -> a.getHitbox().getBoundsInParent().intersects(z.getHitbox().getBoundsInParent()))
                .findFirst().orElse(null);
    }

    private void updateZone(Athlete a) { applyZone(a, detectZone(a)); }

    private void applyZone(Athlete a, TrainingZone dz) {
        if (a.getCurrentZone() != dz) {
            if (a.getCurrentZone() != null) a.getCurrentZone().removeAthlete(a);
            if (dz != null) { dz.addAthlete(a); a.setZoneStatus(true, dz.getEmojiImage()); }
            else              a.setZoneStatus(false, null);
            a.setCurrentZone(dz);
        }
    }

    private static boolean isPointInValidArea(double px, double py) {
        for (TrainingZone zone : zones) {
            if (px >= zone.getX() && px <= zone.getX() + zone.getWidth() &&
                py >= zone.getY() && py <= zone.getY() + zone.getHeight()) {
                return true;
            }
        }
        if (px >= CA_X && px <= FD_X && py >= FD_Y + FD_H / 2.0 - COR_W / 2.0 && py <= FD_Y + FD_H / 2.0 + COR_W / 2.0) return true;
        if (px >= CA_X && px <= CA_X + COR_W && py >= FD_Y + FD_H / 2.0 - COR_W / 2.0 && py <= CA_Y) return true;
        if (px >= FD_X + FD_W && px <= WA_X + WA_W && py >= FD_Y + FD_H / 2.0 - COR_W / 2.0 && py <= FD_Y + FD_H / 2.0 + COR_W / 2.0) return true;
        if (px >= WA_X + WA_W - COR_W && px <= WA_X + WA_W && py >= FD_Y + FD_H / 2.0 - COR_W / 2.0 && py <= WA_Y) return true;
        if (px >= FD_X + FD_W / 2.0 - COR_W / 2.0 && px <= FD_X + FD_W / 2.0 + COR_W / 2.0 && py >= FD_Y + FD_H && py <= CA_Y + CA_H / 2.0 + COR_W / 2.0) return true;
        if (px >= CA_X + CA_W && px <= FD_X + FD_W / 2.0 + COR_W / 2.0 && py >= CA_Y + CA_H / 2.0 - COR_W / 2.0 && py <= CA_Y + CA_H / 2.0 + COR_W / 2.0) return true;
        if (px >= FD_X + FD_W / 2.0 - COR_W / 2.0 && px <= WA_X && py >= WA_Y + WA_H / 2.0 - COR_W / 2.0 && py <= WA_Y + WA_H / 2.0 + COR_W / 2.0) return true;
        return false;
    }

    private static boolean canMoveTo(Athlete a, double nextX, double nextY) {
        double hX = nextX;
        double hY = nextY + Athlete.UI_H;
        double hW = Athlete.IMG_W + 4;
        double hH = Athlete.IMG_H;

        if (!isPointInValidArea(hX, hY)) return false;
        if (!isPointInValidArea(hX + hW, hY)) return false;
        if (!isPointInValidArea(hX, hY + hH)) return false;
        if (!isPointInValidArea(hX + hW, hY + hH)) return false;

        if (solidMode) {
            TrainingZone current = a.getCurrentZone();
            if (current != null) {
                if (hX < current.getX() || hX + hW > current.getX() + current.getWidth() ||
                    hY < current.getY() || hY + hH > current.getY() + current.getHeight()) {
                    return false;
                }
            }
        }
        return true;
    }

    public static void assignToZoneInitially(Athlete a) {
        zones.stream()
                .filter(z -> a.getHitbox().getBoundsInParent().intersects(z.getHitbox().getBoundsInParent()))
                .findFirst()
                .ifPresent(z -> { z.addAthlete(a); a.setZoneStatus(true, z.getEmojiImage()); a.setCurrentZone(z); });
    }
}