package gymworld;

import javafx.animation.AnimationTimer;
import javafx.application.Application;
import javafx.scene.Group;
import javafx.scene.Scene;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyCombination;
import javafx.scene.paint.Color;
import javafx.scene.shape.Rectangle;
import javafx.stage.Stage;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class GymWorld extends Application {

    public static Stage mainStage;
    public static Group rootGroup;
    public static List<Athlete> athletes = new ArrayList<>();
    public static List<TrainingZone> zones = new ArrayList<>();
    private Set<KeyCode> activeKeys = new HashSet<>();

    @Override
    public void start(Stage stage) {
        mainStage = stage;
        rootGroup = new Group();
        Scene scene = new Scene(rootGroup, 1920, 1080);
        scene.setFill(Color.web("#E5E5E5"));

        TrainingZone frontDesk = new TrainingZone("Front Desk", 460, 20, 600, 280, "/gymworld/front_desk.png", "/gymworld/wave.png");
        TrainingZone cardioArea = new TrainingZone("Cardio Area", 40, 440, 480, 320, "/gymworld/cardio_area.png", "/gymworld/lightning.png");
        TrainingZone weightsArea = new TrainingZone("Weights Area", 1000, 440, 480, 320, "/gymworld/weights_area.png", "/gymworld/bicep.png");

        zones.add(frontDesk);
        zones.add(cardioArea);
        zones.add(weightsArea);

        // Нижній Т-подібний коридор
        Rectangle bottomHBase = new Rectangle(520, 540, 480, 100);
        bottomHBase.setFill(Color.web("#A0A0A0"));
        Rectangle centerVBase = new Rectangle(710, 300, 100, 340);
        centerVBase.setFill(Color.web("#A0A0A0"));

        Rectangle bottomHInner = new Rectangle(520, 550, 480, 80);
        bottomHInner.setFill(Color.web("#606060"));
        Rectangle centerVInner = new Rectangle(720, 300, 80, 330);
        centerVInner.setFill(Color.web("#606060"));

        // Лівий Г-подібний коридор
        Rectangle leftHBase = new Rectangle(230, 150, 230, 100);
        leftHBase.setFill(Color.web("#A0A0A0"));
        Rectangle leftVBase = new Rectangle(230, 150, 100, 290);
        leftVBase.setFill(Color.web("#A0A0A0"));

        Rectangle leftHInner = new Rectangle(240, 160, 220, 80);
        leftHInner.setFill(Color.web("#606060"));
        Rectangle leftVInner = new Rectangle(240, 160, 80, 280);
        leftVInner.setFill(Color.web("#606060"));

        // Правий Г-подібний коридор
        Rectangle rightHBase = new Rectangle(1060, 150, 230, 100);
        rightHBase.setFill(Color.web("#A0A0A0"));
        Rectangle rightVBase = new Rectangle(1190, 150, 100, 290);
        rightVBase.setFill(Color.web("#A0A0A0"));

        Rectangle rightHInner = new Rectangle(1060, 160, 220, 80);
        rightHInner.setFill(Color.web("#606060"));
        Rectangle rightVInner = new Rectangle(1200, 160, 80, 280);
        rightVInner.setFill(Color.web("#606060"));

        rootGroup.getChildren().addAll(
                bottomHBase, centerVBase, bottomHInner, centerVInner,
                leftHBase, leftVBase, leftHInner, leftVInner,
                rightHBase, rightVBase, rightHInner, rightVInner,
                frontDesk.getVisualGroup(),
                cardioArea.getVisualGroup(),
                weightsArea.getVisualGroup()
        );

        Athlete testAthlete = new Athlete("Yarik", 20, 80.0, 100.0, new Equipment("Exercise band", 5, "kg"), 200, 480, "Athlete");
        Athlete powerLifter = new Athlete("Big Vasya", 35, 120.0, 100.0, new Equipment("Weight belt", 10, "kg"), 1160, 480, "Powerlifter");
        Athlete gymBoss = new Athlete("Boss Ruslan", 45, 95.0, 100.0, new Equipment("Water bottle", 1, "kg"), 500, 40, "Gym Boss");

        athletes.add(testAthlete);
        athletes.add(powerLifter);
        athletes.add(gymBoss);

        rootGroup.getChildren().addAll(
                testAthlete.getVisualGroup(),
                powerLifter.getVisualGroup(),
                gymBoss.getVisualGroup()
        );

        scene.setOnKeyPressed(event -> {
            activeKeys.add(event.getCode());

            if (event.getCode() == KeyCode.INSERT) {
                activeKeys.remove(KeyCode.INSERT);
                Athlete newAthlete = InsertDialog.display();
                if (newAthlete != null) {
                    athletes.add(newAthlete);
                    rootGroup.getChildren().add(newAthlete.getVisualGroup());
                    newAthlete.getVisualGroup().toFront();
                }
            }

            if (event.isControlDown() && event.getCode() == KeyCode.C) {
                List<Athlete> newClones = new ArrayList<>();
                for (Athlete a : athletes) {
                    if (a.isActive()) {
                        try {
                            Athlete clone = (Athlete) a.clone();
                            newClones.add(clone);
                            rootGroup.getChildren().add(clone.getVisualGroup());
                            clone.getVisualGroup().toFront();
                        } catch (CloneNotSupportedException e) {
                            e.printStackTrace();
                        }
                    }
                }
                athletes.addAll(newClones);
            }

            if (event.getCode() == KeyCode.ESCAPE) {
                for (Athlete a : athletes) {
                    if (a.isActive()) {
                        a.toggleActive();
                    }
                }
            }

            if (event.getCode() == KeyCode.DELETE) {
                for (int i = athletes.size() - 1; i >= 0; i--) {
                    Athlete a = athletes.get(i);
                    if (a.isActive()) {
                        if (a.getCurrentZone() != null) {
                            a.getCurrentZone().removeAthlete(a);
                        }
                        a.removeFromScene();
                        athletes.remove(i);
                    }
                }
            }
        });

        scene.setOnKeyReleased(event -> activeKeys.remove(event.getCode()));

        AnimationTimer timer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                double dx = 0;
                double dy = 0;

                if (activeKeys.contains(KeyCode.UP)) dy = -6;
                if (activeKeys.contains(KeyCode.DOWN)) dy = 6;
                if (activeKeys.contains(KeyCode.LEFT)) dx = -6;
                if (activeKeys.contains(KeyCode.RIGHT)) dx = 6;

                if (dx != 0 || dy != 0) {
                    for (Athlete a : athletes) {
                        if (a.isActive()) {

                            double nextX = a.getX() + dx;
                            double nextY = a.getY() + dy;
                            double actualDx = 0;
                            double actualDy = 0;

                            if (nextX >= 0 && nextX <= scene.getWidth() - 160) actualDx = dx;
                            if (nextY >= 0 && nextY <= scene.getHeight() - 260) actualDy = dy;

                            if (actualDx != 0 || actualDy != 0) {
                                a.move(actualDx, actualDy);

                                TrainingZone detectedZone = null;
                                for (TrainingZone zone : zones) {
                                    if (a.getVisualGroup().getBoundsInParent().intersects(zone.getVisualGroup().getBoundsInParent())) {
                                        detectedZone = zone;
                                        break;
                                    }
                                }

                                if (a.getCurrentZone() != detectedZone) {
                                    if (a.getCurrentZone() != null) {
                                        a.getCurrentZone().removeAthlete(a);
                                    }

                                    if (detectedZone != null) {
                                        detectedZone.addAthlete(a);
                                        a.setZoneStatus(true, detectedZone.getEmojiImage());
                                    } else {
                                        a.setZoneStatus(false, null);
                                    }

                                    a.setCurrentZone(detectedZone);
                                }
                            }
                        }
                    }
                }
            }
        };
        timer.start();

        stage.setTitle("GymWorld");
        stage.setScene(scene);
        stage.setFullScreen(true);
        stage.setFullScreenExitKeyCombination(KeyCombination.NO_MATCH);
        stage.setResizable(false);
        stage.show();
    }
}