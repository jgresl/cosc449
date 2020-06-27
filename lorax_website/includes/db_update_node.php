<?php
   // Get parameters from POST
   $node_ID = $_POST['node_ID'];
   $sample_new = $_POST['sample'];
   $transmit_new = $_POST['transmit'];
   $latitude_new = $_POST['latitude'];
   $longitude_new = $_POST['longitude'];

   include 'db_connection.php';

   // Open the database connection
   try {
      $pdo = openConnection();
   } catch (PDOException $e){
      die($e->getMessage());
   }

   // Query database to find existing node settings
   include '../sql/select_node_settings.php';
   class Node
   {
      public $last_transmission;
      public $sample_old;
      public $transmit_old;
      public $latitude_old;
      public $longitude_old;

   }
   $Node = $statement->fetchObject('Node');

   // Modify node settings in database
   session_start();
   if ($Node->sample_old != $sample_new) {
      include '../sql/update_node_sample.php';
      $_SESSION['message'] = "Sample frequency for node $node_ID has been changed from $Node->sample_old minutes to $sample_new minutes";
   }

   if ($Node->transmit_old != $transmit_new) {
      include '../sql/update_node_transmit.php';
      $_SESSION['message'] = "Transmission frequency for node $node_ID has been changed from $Node->transmit_old minutes to $transmit_new minutes";
   }

   if ($Node->latitude_old != $latitude_new) {
      include '../sql/update_node_latitude.php';
      $_SESSION['message'] = "Latitude for node $node_ID has been changed from $Node->latitude_old to $latitude_new";
   }

   if ($Node->longitude_old != $longitude_new) {
      include '../sql/update_node_longitude.php';
      $_SESSION['message'] = "Longitude for node $node_ID has been changed from $Node->longitude_old to $longitude_new";
   }

   // Close the database connection
   closeConnection($pdo);
   
   // Redirect to new page
   header("Location: ../main.php?command=nodes");

   ?>
