import React from 'react';
import { useSelector } from 'react-redux';
import './log.css';

const Logpage = () => {
  // const [serverStatus, setServerStatus] = useState('');
  // const [addedTorrent, setAddedTorrent] = useState({});
  // const {type, message} = props;

  // useEffect(() => {
  //   const sampleServerStatus = 'Server is listening on 127.0.0.1:8001';
  //   const sampleAddedTorrent = {
  //     announce: 'http://127.0.0.1:8000',
  //     info: {
  //       'piece length': 102400,
  //       pieces: '4e8a7d9f695a62724217a5da8cfc17308e9aa13af7c4b80573152905fb9c1bcb4b22dd856b416caa19785e4628ee9c5bcc7ff50cd21c5cc1d8ac1b',
  //       name: 'thanhml.jpg',
  //       length: 211848
  //     }
  //   };

  //   setServerStatus(sampleServerStatus);
  //   setAddedTorrent(sampleAddedTorrent);
  // }, []);

  const log = useSelector(state => state.logAction);
  console.log(log)

  return (
    <div className="logpage-container">
      <h2>Log</h2>
      <hr className="border" />

      <div className="log-info">
        <div className="server-status">
          
          {Object.entries(log).map((it) => {
            return(
              <>
              {it[1] === null? <></> : 
              <>
              <p className='title_log'>{it[1].title}</p>
            <pre>
              {Array.from(it[1].content.message).map((item) => typeof item === 'string'? (item + '\n') : JSON.stringify(item, null, 2))}
            </pre>
              </>
            }
            </>
            );
          })}
        </div>
      </div>
    </div>
  );
};

export default Logpage;
