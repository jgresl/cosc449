<?php
   session_start();
   // Get parameters from POST
   $gateway_ID = $_POST['gateway_ID'];
   $description_new = $_POST['description'];
   $latitude_new = $_POST['latitude'];
   $longitude_new = $_POST['longitude'];

   include 'db_connection.php';

   // Open the database connection
   try {
      $pdo = openConnection();
   } catch (PDOException $e){
      die($e->getMessage());
   }

   // Query database to find existing Gateway settings
   include '../sql/select_gateway_settings.php';
   class Gateway
   {
      public $description_old;
      public $latitude_old;
      public $longitude_old;
   }
   $Gateway = $statement->fetchObject('Gateway');

   // Modify Gateway settings in database
   if ($Gateway->description_old != $description_new) {
      include '../sql/update_gateway_description.php';
      $_SESSION['message'] = "Description for Gateway $Gateway_ID has been changed from $Gateway->description_old to $description_new";
   }

   if ($Gateway->latitude_old != $latitude_new) {
      include '../sql/update_gateway_latitude.php';
      $_SESSION['message'] = "Latitude for Gateway $Gateway_ID has been changed from $Gateway->latitude_old to $latitude_new";
   }

   if ($Gateway->longitude_old != $longitude_new) {
      include '../sql/update_gateway_longitude.php';
      $_SESSION['message'] = "Longitude for Gateway $Gateway_ID has been changed from $Gateway->longitude_old to $longitude_new";
   }

   // Close the database connection
   closeConnection($pdo);
   
   // Redirect to new page
   header("Location: ../main.php?command=network");
