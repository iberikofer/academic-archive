package gymworld;

import javafx.geometry.Insets;
import javafx.scene.Scene;
import javafx.scene.control.Alert;
import javafx.scene.control.Button;
import javafx.scene.control.ComboBox;
import javafx.scene.control.Label;
import javafx.scene.control.TableView;
import javafx.scene.control.TableColumn;
import javafx.scene.control.cell.PropertyValueFactory;
import javafx.beans.property.SimpleStringProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;
import javafx.scene.layout.VBox;
import javafx.stage.Modality;
import javafx.stage.Stage;

import java.util.Comparator;
import java.util.List;
import java.util.stream.Collectors;

public class FilterDialog {

    public static void display() {
        Stage window = new Stage();
        window.initOwner(GymWorld.mainStage);
        window.initModality(Modality.APPLICATION_MODAL);
        window.setTitle("Filter and Sort Athletes");

        VBox layout = new VBox(10);
        layout.setPadding(new Insets(20));
        layout.getStyleClass().add("dialog-container");

        Label headerLabel = new Label("GYM WORLD QUERIES");
        headerLabel.getStyleClass().add("dialog-header");

        Label filterLabel = new Label("Query A — Filter by Zone:");
        ComboBox<String> filterCombo = new ComboBox<>();
        GymWorld.zones.stream()
                .map(TrainingZone::getName)
                .forEach(filterCombo.getItems()::add);
        filterCombo.getItems().add("Free Objects (None)");
        filterCombo.setValue(filterCombo.getItems().get(0));

        Label sortLabel = new Label("Sort By:");
        ComboBox<String> sortCombo = new ComboBox<>();
        sortCombo.getItems().addAll("Name", "Weight", "Energy Level");
        sortCombo.setValue("Name");

        Button queryAButton = new Button("Run Query A (filter + sort)");
        queryAButton.setOnAction(e -> {
            String selectedFilter = filterCombo.getValue();
            String selectedSort   = sortCombo.getValue();

            List<Athlete> result = GymWorld.athletes.stream()
                    .filter(a -> {
                        if ("Free Objects (None)".equals(selectedFilter)) {
                            return a.getCurrentZone() == null;
                        } else {
                            return a.getCurrentZone() != null
                                    && a.getCurrentZone().getName().equals(selectedFilter);
                        }
                    })
                    .sorted(resolveSortComparator(selectedSort))
                    .collect(Collectors.toList());

            StringBuilder sb = new StringBuilder();
            result.forEach(a -> sb
                    .append("- ").append(a.getName())
                    .append(" [").append(a.getType()).append("] | ")
                    .append("Weight: ").append(a.getWeight()).append(" kg | ")
                    .append("Energy: ").append(String.format("%.1f", a.getEnergyLvl())).append("\n"));

            window.close();
            showInfo("Query A Result",
                    "Filter: " + selectedFilter + " | Sorted by: " + selectedSort,
                    result.isEmpty()
                            ? "No athletes found in this category."
                            : "Found " + result.size() + " athlete(s):\n\n" + sb);
        });

        Button queryBButton = new Button("Run Query B (energy > 50%)");
        queryBButton.setOnAction(e -> {
            long count = GymWorld.athletes.stream()
                    .filter(a -> a.getEnergyLvl() > 50.0)
                    .count();

            String names = GymWorld.athletes.stream()
                    .filter(a -> a.getEnergyLvl() > 50.0)
                    .sorted(Comparator.comparingDouble(Athlete::getEnergyLvl).reversed())
                    .map(a -> a.getName() + " (" + String.format("%.1f", a.getEnergyLvl()) + "%)")
                    .collect(Collectors.joining(", "));

            window.close();
            showInfo("Query B — High Energy Athletes",
                    "Athletes with energy level > 50%",
                    "Count: " + count + "\n\n" + (names.isEmpty() ? "None found." : names));
        });

        Button queryCButton = new Button("Run Query C (right half of world, x >= 1920)");
        queryCButton.setOnAction(e -> {
            long count = GymWorld.athletes.stream()
                    .filter(a -> a.getX() >= 1920)
                    .count();

            String names = GymWorld.athletes.stream()
                    .filter(a -> a.getX() >= 1920)
                    .sorted(Athlete.NameComparator)
                    .map(a -> a.getName() + " (x=" + (int) a.getX() + ")")
                    .collect(Collectors.joining(", "));

            window.close();
            showInfo("Query C — Right Half of World",
                    "Athletes with X coordinate >= 960",
                    "Count: " + count + "\n\n" + (names.isEmpty() ? "None found." : names));
        });

        filterCombo.setMaxWidth(Double.MAX_VALUE);
        sortCombo.setMaxWidth(Double.MAX_VALUE);
        queryAButton.setMaxWidth(Double.MAX_VALUE);
        queryBButton.setMaxWidth(Double.MAX_VALUE);
        queryCButton.setMaxWidth(Double.MAX_VALUE);

        Label tableLabel = new Label("ALL ACTIVE ATHLETES IN THE WORLD:");
        tableLabel.getStyleClass().add("section-label");

        TableView<Athlete> tableView = new TableView<>();
        tableView.setPrefHeight(220);

        TableColumn<Athlete, String> nameCol = new TableColumn<>("Name");
        nameCol.setCellValueFactory(new PropertyValueFactory<>("name"));
        nameCol.setPrefWidth(130);

        TableColumn<Athlete, String> typeCol = new TableColumn<>("Type");
        typeCol.setCellValueFactory(new PropertyValueFactory<>("type"));
        typeCol.setPrefWidth(100);

        TableColumn<Athlete, Double> weightCol = new TableColumn<>("Weight (kg)");
        weightCol.setCellValueFactory(new PropertyValueFactory<>("weight"));
        weightCol.setPrefWidth(90);

        TableColumn<Athlete, Double> energyCol = new TableColumn<>("Energy (%)");
        energyCol.setCellValueFactory(new PropertyValueFactory<>("energyLvl"));
        energyCol.setPrefWidth(90);

        TableColumn<Athlete, String> eqCol = new TableColumn<>("Equipment");
        eqCol.setCellValueFactory(cellData -> {
            Equipment eq = cellData.getValue().getEquipment();
            return new SimpleStringProperty(eq != null ? eq.getName() : "None");
        });
        eqCol.setPrefWidth(125);

        TableColumn<Athlete, String> zoneCol = new TableColumn<>("Current Zone");
        zoneCol.setCellValueFactory(cellData -> {
            TrainingZone zone = cellData.getValue().getCurrentZone();
            return new SimpleStringProperty(zone != null ? zone.getName() : "None (Free)");
        });
        zoneCol.setPrefWidth(125);

        tableView.getColumns().addAll(nameCol, typeCol, weightCol, energyCol, eqCol, zoneCol);

        ObservableList<Athlete> data = FXCollections.observableArrayList(GymWorld.athletes);
        tableView.setItems(data);

        layout.getChildren().addAll(
                headerLabel,
                filterLabel, filterCombo,
                sortLabel, sortCombo,
                queryAButton, queryBButton, queryCButton,
                tableLabel, tableView
        );

        Scene scene = new Scene(layout, 720, 640);
        scene.getStylesheets().add(FilterDialog.class.getResource("/gymworld/style.css").toExternalForm());
        window.setScene(scene);
        window.showAndWait();
    }

    private static Comparator<Athlete> resolveSortComparator(String criterion) {
        return switch (criterion) {
            case "Weight"       -> Athlete.WeightComparator;
            case "Energy Level" -> Athlete.EnergyComparator;
            default             -> Athlete.NameComparator;
        };
    }

    private static void showInfo(String title, String header, String content) {
        Alert alert = new Alert(Alert.AlertType.INFORMATION);
        alert.getDialogPane().getStylesheets().add(FilterDialog.class.getResource("/gymworld/style.css").toExternalForm());
        alert.setTitle(title);
        alert.setHeaderText(header);
        alert.setContentText(content);
        alert.showAndWait();
    }
}