<script type="text/javascript">
  document.addEventListener('DOMContentLoaded', function () {
    const writeApiKey = "PLACE YOUR WRITE API KEY HERE"; // Replace with your Write API Key
    const baseUrl = "https://api.thingspeak.com/update";

    const frequencyInput = document.getElementById('frequency');
    const setFrequencyBtn = document.getElementById('setFrequencyBtn');
    const manualCaptureBtn = document.getElementById('manualCaptureBtn');
    const responseDiv = document.getElementById('response');

    // Manual Image Capture
    manualCaptureBtn.addEventListener('click', function () {
      manualCaptureBtn.disabled = true;
      responseDiv.textContent = 'Triggering manual capture...';

      fetch(`${baseUrl}?api_key=${writeApiKey}&field4=1`)
        .then(response => {
          if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
          }
          return response.text();
        })
        .then(data => {
          responseDiv.textContent = 'Manual capture triggered!';
          console.log(data);
        })
        .catch(error => {
          console.error(error);
          responseDiv.textContent = `Error triggering capture: ${error.message}`;
        })
        .finally(() => {
          manualCaptureBtn.disabled = false;
        });
    });

    // Set Automatic Image Capture Frequency
    setFrequencyBtn.addEventListener('click', function () {
      const frequencyInMinutes = parseInt(frequencyInput.value, 10);
      if (!frequencyInMinutes || isNaN(frequencyInMinutes) || frequencyInMinutes <= 0) {
        alert('Please enter a valid frequency in minutes.');
        return;
      }

      setFrequencyBtn.disabled = true;
      responseDiv.textContent = 'Updating capture frequency...';

      fetch(`${baseUrl}?api_key=${writeApiKey}&field1=${frequencyInMinutes}`)
        .then(response => {
          if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
          }
          return response.text();
        })
        .then(data => {
          responseDiv.textContent = 'Capture frequency updated!';
          console.log(data);
        })
        .catch(error => {
          console.error(error);
          responseDiv.textContent = `Error updating frequency: ${error.message}`;
        })
        .finally(() => {
          setFrequencyBtn.disabled = false;
        });
    });
  });
</script>




