using System;
using System.Text;

static HttpClient client = new HttpClient();
public static async Task Run(string myIoTHubMessage, TraceWriter log)
{
    log.Info($"C# IoT Hub trigger function processed a message: {myIoTHubMessage}");
    var httpContent = new StringContent(myIoTHubMessage, Encoding.UTF8, "application/json");
    var response = await client.PostAsync("https://prod-16.westeurope.logic.azure.com:443/workflows/5252f2a1229442d4857008a8b49f8e26/triggers/manual/paths/invoke?api-version=2016-10-01&sp=%2Ftriggers%2Fmanual%2Frun&sv=1.0&sig=ysWjQnvX3MlUGJxu1ceec8A-uxD-q4geUb2CiUpUmOo", httpContent);

    if (!response.IsSuccessStatusCode)
    {
        // Show the error for the failure.
        var errorMessage = await response.Content.ReadAsStringAsync();
        throw new Exception(
            $"SendGrid failed to send email. Code: {response.StatusCode}, error: {errorMessage}");
    }
}
  