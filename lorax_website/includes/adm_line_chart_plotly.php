<?php
// Retrieve arguments from superglobals
if (isset($_GET['node'])) {
    $nodes = $_GET['node'];
    $sensor_type = $_GET['sensor'];
} else {
    $nodes[0] = 1;
    $sensor_type = "T";
}

// Build class to store result set attributes
class Sample
{
    public $date_time;
    public $value;
}

// Initialize arrays used for populating the chart
$samples = array();

// Create an array for each node_ID selected
foreach ($nodes as $node_ID) {

    // Prepare SQL statement
    $sql = "SELECT date_time, value
    FROM Sample
    WHERE node_ID = :node_ID AND sensor_type = :sensor_type";
    $statement = $pdo->prepare($sql);
    $statement->bindvalue(':node_ID', $node_ID);
    $statement->bindvalue(':sensor_type', $sensor_type);

    // Execute prepared SQL statement
    $statement->execute();

    // Populate arrays with data from result set
    $sample_date_time = array();
    $sample_value = array();
    while ($sample = $statement->fetchObject('Sample')) {
        array_push($sample_date_time, $sample->date_time);
        array_push($sample_value, $sample->value);
    }
    array_push($samples, [$node_ID, [$sample_date_time, $sample_value]]);
}
?>

<div id="column2">
    <article style="height:600px">
        <h1>Time Series Line Chart - plotly</h1><br>

        <div class="filter_row">
            <form method="get" action="main.php">
                <label for="node">Node:</label>
                <input type="hidden" name="command" value="line_chart_plotly"></input>
                <select name="node[]" id="node" multiple size=4>
                    <?php
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

                <label for="sensor">Sensor Type:</label>
                <select name="sensor" id="sensor">
                    <option value="T" <?php if ($sensor_type === "T") echo "selected"; ?>>Temperature</option>
                    <option value="B" <?php if ($sensor_type === "B") echo "selected"; ?>>Battery</option>
                </select>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;

                <input type="submit" style="width: 5em;" value="Refresh">
            </form>
        </div>

        <div id="myDiv"></div>
        <script src='https://cdn.plot.ly/plotly-latest.min.js'></script>
        <script>

            const samples = <?php echo json_encode($samples); ?>;

            console.log(samples);

            var data = [];
            for (var i = 0; i < samples.length; i++) {
                data.push({
                    type: "scatter",
                    mode: "lines",
                    name: 'Node ' + samples[i][0],
                    x: samples[i][1][0],
                    y: samples[i][1][1],
                });
            }

            var layout = {
                xaxis: {
                    autorange: true,
                    rangeselector: {
                        buttons: [{
                                count: 1,
                                label: '1m',
                                step: 'month',
                                stepmode: 'backward'
                            },
                            {
                                count: 6,
                                label: '6m',
                                step: 'month',
                                stepmode: 'backward'
                            },
                            {
                                step: 'all'
                            }
                        ]
                    },
                    rangeslider: {
                        autorange: true
                    },
                    type: 'date'
                },
                yaxis: {
                    autorange: true,
                    type: 'linear'
                },
                showlegend: true
            };

            Plotly.newPlot('myDiv', data, layout);
        </script>
    </article>
</div>