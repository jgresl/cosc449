<%@ page import="java.sql.*"%>
<%@ page contentType="text/html; charset=UTF-8" pageEncoding="UTF8"%>
<!DOCTYPE html>
<html>
<head>
<title>LoRaX Data Server</title>
</head>
<body>
	<%
		String url = "jdbc:mysql://cosc304.ok.ubc.ca/db_jgresl";
		String uid = "jgresl";
		String pw = "29164977";

		try (Connection con = DriverManager.getConnection(url, uid, pw); Statement stmt = con.createStatement();) {
			ResultSet rst = stmt.executeQuery(
					"SELECT reading_id, node_id, sensor_type, date_time, value FROM temperature_data");
			out.println("<table border='1'>");
			out.println("<tr><th>Sample ID</th><th>Node ID</th><th>Sensor Type</th><th>Time Sampled</th><th>Value</th></tr>");
			while (rst.next()) {
				out.println("<tr><td>" + rst.getString(1) + "</td><td>" + rst.getString(2) + "</td><td>"
						+ rst.getString(3) + "</td><td>" + rst.getString(4) + "</td><td>" + rst.getString(5)
						+ "</td></tr>");
			}
			out.println("</table>");
		} catch (SQLException ex) {
			System.err.println("SQLException: " + ex);
		}
	%>
</body>
</html>