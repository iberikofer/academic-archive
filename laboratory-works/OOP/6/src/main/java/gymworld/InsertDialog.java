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
        layout.getStyleClass().add("dialog-container");

        Label headerLabel = new Label("NEW ATHLETE");
        headerLabel.getStyleClass().add("dialog-header");

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

            double spawnX = 1520;
            double spawnY = 200;
            String selectedZone = zoneCombo.getValue();

            if ("Cardio Area".equals(selectedZone)) {
                spawnX = 300;
                spawnY = 1580;
            } else if ("Weights Area".equals(selectedZone)) {
                spawnX = 2940;
                spawnY = 1580;
            }

            String selectedType = typeCombo.getValue();
            if ("Powerlifter".equals(selectedType)) {
                newAthlete = new PowerLifter(name, 25, weight, 100.0, eq, spawnX, spawnY);
            } else if ("Gym Boss".equals(selectedType)) {
                newAthlete = new GymBoss(name, 45, weight, 100.0, eq, spawnX, spawnY);
            } else {
                newAthlete = new Athlete(name, 20, weight, 100.0, eq, spawnX, spawnY);
            }

            window.close();
        });

        typeCombo.setMaxWidth(Double.MAX_VALUE);
        nameInput.setMaxWidth(Double.MAX_VALUE);
        weightInput.setMaxWidth(Double.MAX_VALUE);
        eqCombo.setMaxWidth(Double.MAX_VALUE);
        zoneCombo.setMaxWidth(Double.MAX_VALUE);
        submitButton.setMaxWidth(Double.MAX_VALUE);

        layout.getChildren().addAll(headerLabel, typeLabel, typeCombo, nameLabel, nameInput, weightLabel, weightInput, hasEquipment, eqLabel, eqCombo, zoneLabel, zoneCombo, submitButton);
        Scene scene = new Scene(layout, 350, 540);
        scene.getStylesheets().add(InsertDialog.class.getResource("/gymworld/style.css").toExternalForm());
        window.setScene(scene);
        window.showAndWait();

        return newAthlete;
    }
}