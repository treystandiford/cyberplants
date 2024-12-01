<script type="text/javascript">
  const accessToken = 'PUT YOUR ACCESS TOKEN HERE'; 

  fetch('https://api.imgur.com/3/account/me/images', {
    method: 'GET',
    headers: {
      Authorization: `Bearer ${accessToken}`,
    },
  })
    .then((response) => response.json())
    .then((data) => {
      const images = data.data;
      let latestSideViewImage = null;

      for (let image of images) {
        if (image.title && image.title.startsWith('side_view')) {
          latestSideViewImage = image;
          break;
        }
      }

      if (latestSideViewImage) {
        const imgElement = document.getElementById('imgur-image');
        imgElement.src = latestSideViewImage.link;
        imgElement.alt = 'Latest Side-View Image';

        const timestampElement = document.getElementById('image-timestamp');
        const timestamp = new Date(latestSideViewImage.datetime * 1000).toLocaleString();
        timestampElement.textContent = `Latest Side-View Image: ${timestamp}`;

        document.getElementById('plugin-container').firstElementChild.textContent = 'Latest side-view image:';
      } else {
        console.error('No side-view images found');
        document.getElementById('plugin-container').firstElementChild.textContent = 'No side-view images found.';
      }
    })
    .catch((error) => {
      console.error('Error fetching Imgur data:', error);
      document.getElementById('plugin-container').firstElementChild.textContent =
        'Failed to fetch image.';
    });
</script>