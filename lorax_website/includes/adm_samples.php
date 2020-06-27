<?php
    // Prepared SQL statement to return all samples with the newest on top
    include 'sql/select_samples_new.php';

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
    echo '<div id="column2">';
    echo '<article class="post">';
    echo '<h1>All Samples</h1><br>';
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