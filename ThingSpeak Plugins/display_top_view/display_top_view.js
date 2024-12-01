<script type="text/javascript">
  const accessToken = 'PLACE YOUR ACCESS TOKEN HERE';

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
        if (image.title && image.title.startsWith('top_view')) {
          latestSideViewImage = image;
          break;
        }
      }

      if (latestSideViewImage) {
        const imgElement = document.getElementById('imgur-image');
        imgElement.src = latestSideViewImage.link;
        imgElement.alt = 'Latest Top-View Image';

        const timestampElement = document.getElementById('image-timestamp');
        const timestamp = new Date(latestSideViewImage.datetime * 1000).toLocaleString();
        timestampElement.textContent = `Latest Side-View Image: ${timestamp}`;

        document.getElementById('plugin-container').firstElementChild.textContent = 'Latest top-view image:';
      } else {
        console.error('No top-view images found');
        document.getElementById('plugin-container').firstElementChild.textContent = 'No top-view images found.';
      }
    })
    .catch((error) => {
      console.error('Error fetching Imgur data:', error);
      document.getElementById('plugin-container').firstElementChild.textContent =
        'Failed to fetch image.';
    });
</script>