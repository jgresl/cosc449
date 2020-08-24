<div id="column2">
    <article class="post">
        <h1>Sample List</h1><br>
        <div class="filter_row2">
            <form method="get" action="main.php">
                <label>Node:</label>
                <input type="hidden" name="command" value="samples"></input>
                <select name="node">
                    <?php

                    // Retrieve arguments from superglobals
                    if (isset($_GET['node'])) {
                        $node_ID = $_GET['node'];
                        $sensor_type = $_GET['sensor'];
                    }
                    
                    // Prepare SQL statement
                    $sql = "SELECT node_ID, node_description FROM Node";
                    $statement = $pdo->prepare($sql);

                    // Execute prepared SQL statement
                    $statement->execute();

                    // Build class to store result set attributes
                    class NodeSample
                    {
                        public $node_ID;
                        public $node_description;
                    }

                    // Create option for each node_ID in Sample table
                    while ($node = $statement->fetchObject('NodeSample')) {
                        if ($node->node_ID === $node_ID) {
                            echo "<option value=$node->node_ID selected>$node->node_ID - $node->node_description</option>";
                        } else {
                            echo "<option value=$node->node_ID>$node->node_ID - $node->node_description</option>";
                        }
                    }
                    ?>
                </select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;

                <label>Sensor Type:</label>
                <select name="sensor">
                    <option value="T" <?php if ($sensor_type === "T") echo "selected"; ?>>Temperature</option>
                    <option value="B" <?php if ($sensor_type === "B") echo "selected"; ?>>Battery</option>
                </select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;

                <input type="submit" style="width: 5em;" value="Refresh">
            </form>
        </div><br>
        <?php

        // Retrieve arguments from superglobals
        if (isset($_GET['node'])) {
            $node_ID = $_GET['node'];
            $sensor_type = $_GET['sensor'];
            include 'sql/select_samples_filtered.php';
        } else {
            include 'sql/select_samples_new.php';
        }

        // Build class to store result set attributes
        class Sample
        {
            public $sample_ID;
            public $node_ID;
            public $sensor_type;
            public $date_time;
            public $value;
            public $sample;
        }

        // Display a table with all samples
        $samples = 0;
        echo '<table>';
        echo '<tr><th>sample_ID</th><th>node_ID</th><th>sensor_type</th><th>date_time</th><th>value</th><th>sample</th></tr>';
        while ($sample = $statement->fetchObject('Sample')) {
            echo "<tr><td>$sample->sample_ID</td><td>$sample->node_ID</td><td>$sample->sensor_type</td><td>$sample->date_time</td><td>$sample->value</td><td>$sample->sample</td></tr>";
            $samples++;
        }
        echo '</table>';
        if ($samples == 0)
            echo '<br><h1>There are no samples to display</h1>';
        echo '</article>';
        echo '</div>';
        ?>