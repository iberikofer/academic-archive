package gymworld;

import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

import java.util.ArrayList;
import java.util.List;

public class FilterDialog {
    public static void display() {
        Stage window = new Stage();
        window.initOwner(GymWorld.mainStage);
        window.initModality(Modality.APPLICATION_MODAL);
        window.setTitle("Query Lists (Tasks 9, 10, 11)");

        VBox layout = new VBox(10);
        layout.setPadding(new Insets(20));

        Label filterLabel = new Label("Target Macro-object:");
        ComboBox<String> filterCombo = new ComboBox<>();

        for (TrainingZone zone : GymWorld.zones) {
            filterCombo.getItems().add(zone.getName());
        }
        filterCombo.getItems().add("Free Objects (None)");
        filterCombo.setValue("Front Desk");

        Label sortLabel = new Label("Sort By (3 Criteria):");
        ComboBox<String> sortCombo = new ComboBox<>();
        sortCombo.getItems().addAll("Name", "Weight", "Energy Level");
        sortCombo.setValue("Name");

        Button executeButton = new Button("Execute Query");
        executeButton.setOnAction(e -> {
            String selectedFilter = filterCombo.getValue();
            String selectedSort = sortCombo.getValue();

            List<Athlete> queryResult = new ArrayList<>();

            for (Athlete a : GymWorld.athletes) {
                if ("Free Objects (None)".equals(selectedFilter)) {
                    if (a.getCurrentZone() == null) {
                        queryResult.add(a);
                    }
                } else {
                    if (a.getCurrentZone() != null && a.getCurrentZone().getName().equals(selectedFilter)) {
                        queryResult.add(a);
                    }
                }
            }

            if ("Name".equals(selectedSort)) {
                queryResult.sort(Athlete.NameComparator);
            } else if ("Weight".equals(selectedSort)) {
                queryResult.sort(Athlete.WeightComparator);
            } else if ("Energy Level".equals(selectedSort)) {
                queryResult.sort(Athlete.EnergyComparator);
            }

            StringBuilder outputText = new StringBuilder();
            for (Athlete a : queryResult) {
                outputText.append("- ").append(a.getName())
                        .append(" [").append(a.getType()).append("] | ")
                        .append("Weight: ").append(a.getWeight()).append("kg | ")
                        .append("Energy: ").append(String.format("%.1f", a.getEnergyLvl())).append("\n");
            }

            window.close();

            Alert alert = new Alert(Alert.AlertType.INFORMATION);
            alert.setTitle("Query Result");
            alert.setHeaderText("Filter: " + selectedFilter + " | Sorted by: " + selectedSort);

            if (!queryResult.isEmpty()) {
                alert.setContentText("Found " + queryResult.size() + " athlete(s):\n\n" + outputText.toString());
            } else {
                alert.setContentText("No athletes found in this category.");
            }
            alert.showAndWait();
        });

        layout.getChildren().addAll(filterLabel, filterCombo, sortLabel, sortCombo, executeButton);
        Scene scene = new Scene(layout, 320, 240);
        window.setScene(scene);
        window.showAndWait();
    }
}