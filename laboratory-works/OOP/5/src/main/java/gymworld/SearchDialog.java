package gymworld;

import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TextField;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

public class SearchDialog {
    public static void display() {
        Stage window = new Stage();
        window.initOwner(GymWorld.mainStage);
        window.initModality(Modality.APPLICATION_MODAL);
        window.setTitle("Search Athlete");

        VBox layout = new VBox(10);
        layout.setPadding(new Insets(20));

        Label nameLabel = new Label("Enter Athlete Name:");
        TextField nameInput = new TextField();

        Button searchButton = new Button("Search");
        searchButton.setOnAction(e -> {
            String targetName = nameInput.getText().trim();
            boolean found = false;
            Athlete foundAthlete = null;

            for (Athlete a : GymWorld.athletes) {
                if (a.getName().toLowerCase().contains(targetName.toLowerCase())) {
                    found = true;
                    foundAthlete = a;
                    break;
                }
            }

            window.close();

            if (found) {
                String zoneName = (foundAthlete.getCurrentZone() != null) ? foundAthlete.getCurrentZone().getName() : "None (Free Object)";

                Alert alert = new Alert(Alert.AlertType.INFORMATION);
                alert.setTitle("Search Result");
                alert.setHeaderText("Athlete Found!");
                alert.setContentText("Name: " + foundAthlete.getName() + "\n" +
                        "Class Type: " + foundAthlete.getType() + "\n" +
                        "Location: X = " + foundAthlete.getX() + ", Y = " + foundAthlete.getY() + "\n" +
                        "Macro-object (Zone): " + zoneName);
                alert.showAndWait();
            } else {
                Alert alert = new Alert(Alert.AlertType.WARNING);
                alert.setTitle("Search Result");
                alert.setHeaderText("Not Found");
                alert.setContentText("Athlete with name '" + targetName + "' was not found.");
                alert.showAndWait();
            }
        });

        layout.getChildren().addAll(nameLabel, nameInput, searchButton);
        Scene scene = new Scene(layout, 300, 150);
        window.setScene(scene);
        window.showAndWait();
    }
}