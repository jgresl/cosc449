<!DOCTYPE html>
<html>

<head lang="en">
   <meta charset="utf-8">
   <title>LoRaX Gateway</title>
   <link rel="icon" href="images/favicon.png" type="image/x-icon">
   <link rel="stylesheet" href="css/reset.css">
   <link rel="stylesheet" href="css/main.css">
   <link rel="stylesheet" href="css/header.css">
   <link rel="stylesheet" href="css/line_chart.css">
   <script type="text/javascript" src="js/admin_modify_node.js"></script>
   <script type="text/javascript" src="js/admin_restore_db.js"></script>
   <script async defer src="https://maps.googleapis.com/maps/api/js?key=AIzaSyD_ob_YSzpkQwwXup8Xwc2M9qT6-jytWJI&callback=initMap"></script>
</head>

<body>
   <header>
      <?php include 'includes/header.php'; ?>
   </header>
   <main>
      <div id="column1">
         <article id="profile">
            <div class="banner">
               <h1>Admin</h1>
            </div>
            <br>
            <h3>Maintenance</h3>
            <ul>
               <li><a href="main.php?command=network">Manage Network</a></li>
            </ul>
            <br><br>
            <h3>Reports</h3>
            <ul>
               <li><a href="main.php?command=samples">Sample List</a></li>
               <li><a href="main.php?command=line_chart">Line Chart</a></li>
            </ul>
            <br><br>
            <h3>Database</h3>
            <ul>
               <li><a href="main.php?command=diagram">ER Diagram</a></li>
               <li><a href="main.php?command=ddl">DDL</a></li>
               <li><a href="main.php?command=restore">Database Restore</a></li>
            </ul>
            <br>
         </article>
      </div>
      <?php

      // Open the database connection
      include 'includes/db_connection.php';
      try {
         $pdo = openConnection();
      } catch (PDOException $e) {
         die($e->getMessage());
      }

      if (isset($_GET['command'])) {
         $command = $_GET['command'];
         switch ($command) {
            case "network":
               include 'includes/adm_network.php';
               break;
            case "samples":
               include 'includes/adm_samples.php';
               break;
            case "line_chart":
               include 'includes/adm_line_chart.php';
               break;
            case "diagram":
               include 'includes/adm_diagram.php';
               break;
            case "ddl":
               include 'includes/adm_ddl.php';
               break;
            case "restore":
               include 'includes/adm_restore.php';
               break;
            default:
               include 'includes/adm_samples.php';
               break;
         }
      } else {
         include 'includes/adm_samples.php';
      }

      // Close the database connection
      closeConnection($pdo);
      ?>
   </main>
</body>

</html>