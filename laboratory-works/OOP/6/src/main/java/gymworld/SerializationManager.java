package gymworld;

import javafx.stage.FileChooser;

import java.io.*;
import java.util.ArrayList;
import java.util.List;

public class SerializationManager {


    public static void save() {
        FileChooser chooser = new FileChooser();
        chooser.setTitle("Save GymWorld State");
        chooser.getExtensionFilters().add(
                new FileChooser.ExtensionFilter("GymWorld Save File (*.gym)", "*.gym"));
        chooser.setInitialFileName("gymworld_save.gym");

        File file = chooser.showSaveDialog(GymWorld.mainStage);
        if (file == null) return;

        try (ObjectOutputStream oos = new ObjectOutputStream(new FileOutputStream(file))) {

            oos.writeObject(new ArrayList<>(GymWorld.zones));
            oos.writeObject(new ArrayList<>(GymWorld.athletes));

            showInfo("Save Successful",
                    "World state saved to:\n" + file.getAbsolutePath());

        } catch (IOException ex) {
            showError("Save Failed", ex.getMessage());
        }
    }


    public static void load() {
        FileChooser chooser = new FileChooser();
        chooser.setTitle("Load GymWorld State");
        chooser.getExtensionFilters().add(
                new FileChooser.ExtensionFilter("GymWorld Save File (*.gym)", "*.gym"));

        File file = chooser.showOpenDialog(GymWorld.mainStage);
        if (file == null) return;

        try (ObjectInputStream ois = new ObjectInputStream(new FileInputStream(file))) {

            List<TrainingZone> loadedZones    = (List<TrainingZone>) ois.readObject();
            List<Athlete>      loadedAthletes = (List<Athlete>)      ois.readObject();

            for (Athlete a : GymWorld.athletes) {
                GymWorld.rootGroup.getChildren().remove(a.getVisualGroup());
            }
            for (TrainingZone z : GymWorld.zones) {
                GymWorld.rootGroup.getChildren().remove(z.getVisualGroup());
            }
            GymWorld.athletes.clear();
            GymWorld.zones.clear();
            if (GymWorld.minimap != null) {
                GymWorld.minimap.reset();
            }

            for (TrainingZone zone : loadedZones) {
                zone.initVisuals();
                GymWorld.zones.add(zone);
                GymWorld.rootGroup.getChildren().add(zone.getVisualGroup());
            }

            for (Athlete athlete : loadedAthletes) {
                athlete.initVisuals();

                TrainingZone zone = athlete.getCurrentZone();
                if (zone != null) {
                    athlete.setZoneStatus(true, zone.getEmojiImage());
                }

                GymWorld.athletes.add(athlete);
                GymWorld.rootGroup.getChildren().add(athlete.getVisualGroup());
            }

            showInfo("Load Successful",
                    "World state loaded from:\n" + file.getAbsolutePath());

        } catch (IOException | ClassNotFoundException ex) {
            showError("Load Failed", ex.getMessage());
        }
    }


    private static void showInfo(String title, String message) {
        javafx.scene.control.Alert alert =
                new javafx.scene.control.Alert(javafx.scene.control.Alert.AlertType.INFORMATION);
        alert.setTitle(title);
        alert.setHeaderText(null);
        alert.setContentText(message);
        alert.showAndWait();
    }

    private static void showError(String title, String message) {
        javafx.scene.control.Alert alert =
                new javafx.scene.control.Alert(javafx.scene.control.Alert.AlertType.ERROR);
        alert.setTitle(title);
        alert.setHeaderText("An error occurred");
        alert.setContentText(message);
        alert.showAndWait();
    }
}