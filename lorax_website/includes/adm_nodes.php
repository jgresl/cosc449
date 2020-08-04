<div id="column2">
    <article style="height:550px">
        <h1>Node Map</h1><br>
        <div id="map"></div>
    </article>

    <?php

    // Prepare SQL statement
    $sql = "SELECT node_ID, gps_latitude, gps_longitude FROM Node";
    $statement = $pdo->prepare($sql);

    // Execute prepared SQL statement
    $statement->execute();

    // Build class to store result set attributes
    class NodeLocation
    {
        public $node_ID;
        public $gps_latitude;
        public $gps_longitude;
    }

    // Initialize array used for placcing nodes on the map
    $node_locations = array();

    // Populate arrays with data from result set
    while ($node = $statement->fetchObject('NodeLocation')) {
        array_push($node_locations, [$node->node_ID, $node->gps_latitude, $node->gps_longitude]);
    }
    ?>

    <script>
        function initMap() {
            var gateway = {
                lat: 49.859230,
                lng: -119.605255
            };

            // Get array from PHP variable
            var nodes = <?php echo json_encode($node_locations); ?>;

            var map = new google.maps.Map(document.getElementById('map'), {
                zoom: 20,
                center: gateway,
                mapTypeId: 'satellite',
            });

            var gateway_marker = new google.maps.Marker({
                position: gateway,
                label: 'G',
                map: map,
            });

            // Generate markers for each node.
            for (i = 0; i < nodes.length; i++) {
                marker = new google.maps.Marker({
                    position: new google.maps.LatLng(nodes[i][1], nodes[i][2]),
                    label: nodes[i][0],
                    map: map
                });
            }

            // Create the initial InfoWindow.
            var infoWindow = new google.maps.InfoWindow({
                content: 'Click the map to get Lat/Lng!',
                position: new google.maps.LatLng(49.859288, -119.605258)
            });
            infoWindow.open(map);

            // Configure the click listener.
            map.addListener('click', function(mapsMouseEvent) {
                // Close the current InfoWindow.
                infoWindow.close();

                // Create a new InfoWindow.
                infoWindow = new google.maps.InfoWindow({
                    position: mapsMouseEvent.latLng
                });
                var latitude = mapsMouseEvent.latLng.lat().toFixed(6).toString();
                var longitude = mapsMouseEvent.latLng.lng().toFixed(6).toString();
                infoWindow.setContent(latitude + "     " + longitude);
                infoWindow.open(map);
            });
        }
    </script>




    <?php
    // Prepared SQL statement to return all Nodes with the newest on top
    include 'sql/select_nodes.php';

    // Build class to store result set attributes
    class Node
    {
        public $node_ID;
        public $serial_number;
        public $sync_status;
        public $last_sync;
        public $last_transmission;
        public $sample_frequency;
        public $transmission_frequency;
        public $gps_latitude;
        public $gps_longitude;
        public $status;
    }

    echo '<div id="column2">';

    // Display a message if one exists
    session_start();
    if (isset($_SESSION['message'])) {
        echo '<article class="post">';
        echo '<p class="message">' . $_SESSION['message'] . '</p>';
        echo '</article>';
    }

    // Display a table with all Nodes
    $Nodes = 0;
    echo '<article class="post">';
    echo '<h1>Manage Nodes</h1><br>';
    echo '<table>';
    echo '<tr><th>node_ID</th><th>sample</th><th>transmit</th><th>gps_latitude</th><th>gps_longitude</th><th>sync_status</th><th>last_sync</th><th>last_transmission</th><th>serial_number</th></tr>';
    while ($Node = $statement->fetchObject('Node')) {
        if (strcmp($Node->status, "ON") == 0) {
            echo "<tr class='node_online'>";
        } else {
            echo "<tr class='node_offline'>";
        }
        echo "<form method='post' action='includes/db_update_node.php'>
            <td>$Node->node_ID - $Node->status<input type='hidden' name='node_ID' value='$Node->node_ID'></td>
            <td><input class='edit_field' name='sample' type='number' id='$Node->node_ID' value='$Node->sample_frequency'></input></td>
            <td><input class='edit_field' name='transmit' type='number' id='$Node->node_ID' value='$Node->transmission_frequency'></input></td>
            <td><input name='latitude' type='text' id='$Node->node_ID' value='$Node->gps_latitude'></input></td>
            <td><input class='edit_field' name='longitude' type='text' id='$Node->node_ID' value='$Node->gps_longitude'></input></td>";
            if (strcmp($Node->sync_status, "Required") == 0) {
                echo "<td class='sync_required'>$Node->sync_status</td>";
            } else {
                echo "<td>$Node->sync_status</td>";
            }
            echo "<td>$Node->last_sync</td>
            <td>$Node->last_transmission</td>
            <td>$Node->serial_number</td>
        </form></tr>";
        $Nodes++;
    }
    echo '</table>';
    if ($Nodes == 0)
        echo '<br><h1>There are no Nodes to display</h1>';
    echo '</article>';
    // Clear the message
    unset($_SESSION['message']);
    ?>
</div>