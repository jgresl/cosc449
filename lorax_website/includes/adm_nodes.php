<?php
// Prepared SQL statement to return all Nodes with the newest on top
include 'sql/select_nodes.php';

// Build class to store result set attributes
class Node
{
    public $node_ID;
    public $serial_number;
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
echo '<tr><th>node_ID</th><th>serial_number</th><th>last_sync</th><th>last_transmission</th><th>sample</th><th>transmit</th><th>latitude</th><th>longitude</th></tr>';
while ($Node = $statement->fetchObject('Node')) {
    if (strcmp($Node->status,"ON") == 0) {
        echo "<tr class='node_online'>";
    } else {
        echo "<tr class='node_offline'>";
    }
    echo "<form method='post' action='includes/db_update_node.php'>
            <td>$Node->node_ID - $Node->status<input type='hidden' name='node_ID' value='$Node->node_ID'></td>
            <td>$Node->serial_number</td>
            <td>$Node->last_sync</td>
            <td>$Node->last_transmission</td>
            <td><input class='edit_field' name='sample' type='number' id='$Node->node_ID' value='$Node->sample_frequency'></input></td>
            <td><input class='edit_field' name='transmit' type='number' id='$Node->node_ID' value='$Node->transmission_frequency'></input></td>
            <td><input class='edit_field' name='latitude' type='text' id='$Node->node_ID' value='$Node->gps_latitude'></input></td>
            <td><input class='edit_field' name='longitude' type='text' id='$Node->node_ID' value='$Node->gps_longitude'></input></td>
        </form></tr>";
    $Nodes++;
}
echo '</table>';
if ($Nodes == 0)
    echo '<br><h1>There are no Nodes to display</h1>';
echo '</article>';
echo '</div>';

// Clear the message
unset($_SESSION['message']);
