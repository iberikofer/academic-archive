package gymworld;

import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.CheckBox;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class InsertDialog {
    private static Athlete newAthlete = null;

    public static Athlete display() {
        newAthlete = null;
        Stage window = new Stage();
        window.initOwner(GymWorld.mainStage);
        window.initModality(Modality.APPLICATION_MODAL);
        window.setTitle("New Athlete");

        VBox layout = new VBox(10);
        layout.setPadding(new Insets(20));

        Label typeLabel = new Label("Type:");
        ComboBox<String> typeCombo = new ComboBox<>();
        typeCombo.getItems().addAll("Athlete", "Powerlifter", "Gym Boss");
        typeCombo.setValue("Athlete");

        Label nameLabel = new Label("Name:");
        TextField nameInput = new TextField("New Gym Bro");

        Label weightLabel = new Label("Weight (kg):");
        TextField weightInput = new TextField("80.0");

        CheckBox hasEquipment = new CheckBox("Has Equipment");
        hasEquipment.setSelected(true);

        Label eqLabel = new Label("Equipment:");
        ComboBox<String> eqCombo = new ComboBox<>();
        eqCombo.getItems().addAll("Water bottle", "Dumbbell", "Barbell", "Kettlebell", "Jump Rope", "Exercise Band", "Treadmill", "Elliptical machine", "Stationary bike", "Rowing machine");
        eqCombo.setValue("Exercise band");

        hasEquipment.setOnAction(e -> eqCombo.setDisable(!hasEquipment.isSelected()));

        Label zoneLabel = new Label("Spawn Zone:");
        ComboBox<String> zoneCombo = new ComboBox<>();
        zoneCombo.getItems().addAll("Front Desk", "Cardio Area", "Weights Area");
        zoneCombo.setValue("Front Desk");

        Button submitButton = new Button("Create Athlete");
        submitButton.setOnAction(e -> {
            String name = nameInput.getText();
            double weight = 80.0;
            try {
                weight = Double.parseDouble(weightInput.getText());
            } catch (NumberFormatException ex) {
                weight = 80.0;
            }

            Equipment eq = null;
            if (hasEquipment.isSelected()) {
                eq = new Equipment(eqCombo.getValue(), 5, "kg");
            }

            double spawnX = 680;
            double spawnY = 30;
            String selectedZone = zoneCombo.getValue();

            if ("Cardio Area".equals(selectedZone)) {
                spawnX = 200;
                spawnY = 490;
            } else if ("Weights Area".equals(selectedZone)) {
                spawnX = 1160;
                spawnY = 490;
            }

            newAthlete = new Athlete(name, 20, weight, 100.0, eq, spawnX, spawnY, typeCombo.getValue());
            window.close();
        });

        layout.getChildren().addAll(typeLabel, typeCombo, nameLabel, nameInput, weightLabel, weightInput, hasEquipment, eqLabel, eqCombo, zoneLabel, zoneCombo, submitButton);
        Scene scene = new Scene(layout, 300, 420);
        window.setScene(scene);
        window.showAndWait();

        return newAthlete;
    }
}