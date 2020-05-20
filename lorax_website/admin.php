<!DOCTYPE html>
<html>

<head lang="en">
   <meta charset="utf-8">
   <title>Scratch Post</title>
   <link rel="icon" href="images/favicon.png" type="image/x-icon">
   <link rel="stylesheet" href="css/reset.css">
   <link rel="stylesheet" href="css/main.css">
   <link rel="stylesheet" href="css/header.css">
   <script type="text/javascript" src="js/admin_modify_user.js"></script>
   <script type="text/javascript" src="https://www.gstatic.com/charts/loader.js"></script>
</head>

<body>
   <header>
      <?php include 'includes/header.php'; ?>
      <?php
         // Go to main page if the user does not have admin privileges
         if(!isset($_SESSION['user_type']) || strcmp($_SESSION['user_type'], "Admin") != 0) {
            header("Location: main.php");
         }
      ?>
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
               <li><a href="admin.php?command=users">Manage Users</a></li>
               <li><a href="admin.php?command=posts">Disabled Posts</a></li>
            </ul>
            <br><br>
            <h3>Reports</h3>
            <ul>
               <li><a href="admin.php?command=usage">Site Usage</a></li>
               <li><a href="admin.php?command=activity">Site Activity</a></li>
               <li><a href="admin.php?command=ages">Demographics</a></li>
            </ul>
            <br><br>
            <h3>Database</h3>
            <ul>
               <li><a href="admin.php?command=diagram">ER Diagram</a></li>
               <li><a href="admin.php?command=restore">Database Restore</a></li>
            </ul>
            <br>
         </article>
      </div>
      <div id="column2">

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
               switch($command) {
                  case "users" : 
                     include 'includes/adm_users.php';
                     break;
                  case "posts" :
                     include 'includes/adm_disabled.php';
                     break;
                  case "usage" :
                     include 'includes/adm_usage.php';
                     break;
                  case "activity" :
                     include 'includes/adm_activity.php';
                     break;
                  case "ages" :
                     include 'includes/adm_demographics.php';;
                     break;
                  case "diagram" :
                     include 'includes/adm_diagram.php';;
                     break;
                  case "restore" :
                     include 'includes/adm_restore.php';;
                     break; 
               }
            }

            // Close the database connection
            closeConnection($pdo);
         ?>

      </div>
   </main>
</body>

</html>