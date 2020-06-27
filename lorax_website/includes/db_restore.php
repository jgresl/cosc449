<?php
   include 'db_connection.php';

   // Open the database connection
   try {
      $pdo = openConnection();
      include 'sql/truncate_nodes.php';
   } catch (PDOException $e){
      die($e->getMessage());
   }

   // Restore the database
   include '../sql/truncate_samples.php';
   include '../sql/truncate_nodes.php';

   // Close the database connection
   closeConnection($pdo);
   
   // Redirect to new page
   session_start();
   $_SESSION['message'] = "The database has been restored";
   header("Location: ../main.php?command=nodes");
