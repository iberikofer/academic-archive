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

import java.util.Optional;

public class SearchDialog {

    public static void display() {
        Stage window = new Stage();
        window.initOwner(GymWorld.mainStage);
        window.initModality(Modality.APPLICATION_MODAL);
        window.setTitle("Search Athlete");

        VBox layout = new VBox(10);
        layout.setPadding(new Insets(20));
        layout.getStyleClass().add("dialog-container");

        Label headerLabel = new Label("SEARCH ATHLETE");
        headerLabel.getStyleClass().add("dialog-header");

        Label nameLabel  = new Label("Enter Athlete Name:");
        TextField nameInput = new TextField();

        Button searchButton = new Button("Search");
        searchButton.setOnAction(e -> {
            String targetName = nameInput.getText().trim().toLowerCase();

            Optional<Athlete> found = GymWorld.athletes.stream()
                    .filter(a -> a.getName().toLowerCase().contains(targetName))
                    .findFirst();

            window.close();

            if (found.isPresent()) {
                Athlete a = found.get();
                String zoneName = (a.getCurrentZone() != null)
                        ? a.getCurrentZone().getName()
                        : "None (Free Object)";

                Alert alert = new Alert(Alert.AlertType.INFORMATION);
                alert.getDialogPane().getStylesheets().add(SearchDialog.class.getResource("/gymworld/style.css").toExternalForm());
                alert.setTitle("Search Result");
                alert.setHeaderText("Athlete Found!");
                alert.setContentText(
                        "Name: "               + a.getName()   + "\n" +
                                "Class Type: "         + a.getType()   + "\n" +
                                "Location: X = "       + (int) a.getX() + ", Y = " + (int) a.getY() + "\n" +
                                "Macro-object (Zone): " + zoneName);
                alert.showAndWait();
            } else {
                Alert alert = new Alert(Alert.AlertType.WARNING);
                alert.getDialogPane().getStylesheets().add(SearchDialog.class.getResource("/gymworld/style.css").toExternalForm());
                alert.setTitle("Search Result");
                alert.setHeaderText("Not Found");
                alert.setContentText("Athlete with name '" + targetName + "' was not found.");
                alert.showAndWait();
            }
        });

        nameInput.setMaxWidth(Double.MAX_VALUE);
        searchButton.setMaxWidth(Double.MAX_VALUE);

        layout.getChildren().addAll(headerLabel, nameLabel, nameInput, searchButton);
        Scene scene = new Scene(layout, 350, 200);
        scene.getStylesheets().add(SearchDialog.class.getResource("/gymworld/style.css").toExternalForm());
        window.setScene(scene);
        window.showAndWait();
    }
}