function restore_database() {
    var confirmed = confirm("Are you sure you want to restore the database?");
    if (confirmed)
        location.href = 'includes/db_restore.php';
}