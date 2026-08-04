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

public class EditDialog {
    public static void display(Athlete athlete) {
        Stage window = new Stage();
        window.initOwner(GymWorld.mainStage);
        window.initModality(Modality.APPLICATION_MODAL);
        window.setTitle("Edit Athlete");

        VBox layout = new VBox(10);
        layout.setPadding(new Insets(20));
        layout.getStyleClass().add("dialog-container");

        Label headerLabel = new Label("EDIT ATHLETE");
        headerLabel.getStyleClass().add("dialog-header");

        Label typeLabel = new Label("Type:");
        ComboBox<String> typeCombo = new ComboBox<>();
        typeCombo.getItems().addAll("Athlete", "PowerLifter", "GymBoss");
        typeCombo.setValue(athlete.getType());
        typeCombo.setDisable(true);

        Label nameLabel = new Label("Name:");
        TextField nameInput = new TextField(athlete.getName());

        Label weightLabel = new Label("Weight (kg):");
        TextField weightInput = new TextField(String.valueOf(athlete.getWeight()));

        Label energyLabel = new Label("Energy Level (1-100):");
        TextField energyInput = new TextField(String.valueOf(athlete.getEnergyLvl()));

        CheckBox hasEquipment = new CheckBox("Has Equipment");
        hasEquipment.setSelected(athlete.getEquipment() != null);

        Label eqLabel = new Label("Equipment:");
        ComboBox<String> eqCombo = new ComboBox<>();
        eqCombo.getItems().addAll("Water bottle", "Dumbbell", "Barbell", "Kettlebell", "Jump Rope", "Exercise Band", "Treadmill", "Elliptical machine", "Stationary bike", "Rowing machine");

        if (athlete.getEquipment() != null) {
            eqCombo.setValue(athlete.getEquipment().getName());
        } else {
            eqCombo.setValue("Exercise Band");
            eqCombo.setDisable(true);
        }

        hasEquipment.setOnAction(e -> eqCombo.setDisable(!hasEquipment.isSelected()));

        Button submitButton = new Button("Save Changes");
        submitButton.setOnAction(e -> {
            athlete.setName(nameInput.getText());

            try {
                athlete.setWeight(Double.parseDouble(weightInput.getText()));
            } catch (NumberFormatException ex) {
                athlete.setWeight(80.0);
            }

            try {
                double energy = Double.parseDouble(energyInput.getText());
                if (energy < 1.0) energy = 1.0;
                if (energy > 100.0) energy = 100.0;
                athlete.setEnergyLvl(energy);
            } catch (NumberFormatException ex) {
            }

            if (hasEquipment.isSelected()) {
                athlete.setEquipment(new Equipment(eqCombo.getValue(), 5, "kg"));
            } else {
                athlete.setEquipment(null);
            }

            window.close();
        });

        typeCombo.setMaxWidth(Double.MAX_VALUE);
        nameInput.setMaxWidth(Double.MAX_VALUE);
        weightInput.setMaxWidth(Double.MAX_VALUE);
        energyInput.setMaxWidth(Double.MAX_VALUE);
        eqCombo.setMaxWidth(Double.MAX_VALUE);
        submitButton.setMaxWidth(Double.MAX_VALUE);

        layout.getChildren().addAll(headerLabel, typeLabel, typeCombo, nameLabel, nameInput, weightLabel, weightInput, energyLabel, energyInput, hasEquipment, eqLabel, eqCombo, submitButton);

        Scene scene = new Scene(layout, 350, 580);
        scene.getStylesheets().add(EditDialog.class.getResource("/gymworld/style.css").toExternalForm());
        window.setScene(scene);
        window.showAndWait();
    }
}