<!DOCTYPE html>
<html>

<head lang="en">
   <meta charset="utf-8">
   <title>LoRaX Gateway</title>
   <link rel="icon" href="images/favicon.png" type="image/x-icon">
   <link rel="stylesheet" href="css/reset.css">
   <link rel="stylesheet" href="css/main.css">
   <link rel="stylesheet" href="css/header.css">
</head>

<body>
   <header>
      <?php include 'includes/header.php'; ?>
   </header>
   <main>
            <?php
               // Open the database connection
               try {
                  $pdo = openConnection();
               } catch (PDOException $e) {
                  die($e->getMessage());
               }

               // Display a table with all samples
               include 'sql/select_samples.php';
              
               // Execute prepared SQL statement and store the result set
               $statement->execute();

               // Build class to store result set attributes
               class Sample {
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
                     echo '<table>';
                        echo '<tr><th>sample_ID</th><th>node_ID</th><th>sensor_type</th><th>date_time</th><th>value</th><th>sample</th></tr>';
                        while ($sample = $statement->fetchObject('Sample')) {
                           echo "<tr><td>$sample_ID</td><td>$node_ID</td><td>$sensor_type</td><td>$date_time</td><td>$value</td><td>$sample</td></tr>";
                           $samples++;
                        }
                     echo '</table>';
                  echo '</article>';

                  if ($samples == 0) {
                     echo '<article>';
                        echo '<p>There are no samples to display</p>';
                     echo '</article>';
                  }
               
               echo '</div>';

               // Close the database connection
               closeConnection($pdo);
            ?>
      </div>
   </main>
</body>

</html>