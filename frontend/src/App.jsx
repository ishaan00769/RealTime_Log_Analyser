import { useState, useEffect } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';
import { Activity, ShieldAlert, Globe } from 'lucide-react';
import './App.css';

function App() {
  const [visitors, setVisitors] = useState([]);
  const [domains, setDomains] = useState([]);
  const [suspicious, setSuspicious] = useState([]);
  const [selectedDomain, setSelectedDomain] = useState('');

  useEffect(() => {
    // Define the fetch logic
    const fetchData = async () => {
      try {
        const [visRes, domRes, suspRes] = await Promise.all([
          fetch('http://127.0.0.1:8080/api/visitors'),
          fetch('http://127.0.0.1:8080/api/domains'),
          fetch('http://127.0.0.1:8080/api/suspicious')
        ]);

        const visData = await visRes.json();
        const domData = await domRes.json();
        const suspData = await suspRes.json();

        setVisitors(visData);
        setDomains(domData);
        setSuspicious(suspData);

        // Auto-select the first available domain only if one isn't selected yet
        if (domData.length > 0 && !selectedDomain) {
          const uniqueDomains = [...new Set(domData.map(item => item.domain))];
          setSelectedDomain(uniqueDomains[0]);
        }
      } catch (error) {
        console.error("Failed to fetch from C++ Backend:", error);
      }
    };

    // 1. Fetch immediately on load
    fetchData();

    // 2. Set up the Real-Time Polling Heartbeat (Every 1000ms)
    const heartbeat = setInterval(fetchData, 1000);

    // 3. Clean up the interval on unmount
    return () => clearInterval(heartbeat);
    
  }, [selectedDomain]); // Added selectedDomain to dependency array so it doesn't overwrite user selection

  // Filter the domain data for the specific website chosen in the dropdown
  const uniqueDomainsList = [...new Set(domains.map(item => item.domain))];
  const chartDataForDomain = domains.filter(d => d.domain === selectedDomain);

  return (
    <div className="dashboard">
      <header className="header">
        <Activity className="icon-main" />
        <h1>Real-Time Log Analytics</h1>
      </header>

      <div className="grid-container">
        {/* Panel 1: Unique Visitors (HyperLogLog) */}
        <div className="card">
          <h2><Globe className="icon-small" /> Unique Hourly Visitors (HLL)</h2>
          <div className="chart-container">
            <ResponsiveContainer width="100%" height="100%">
              <LineChart data={visitors}>
                <CartesianGrid strokeDasharray="3 3" stroke="#333" />
                <XAxis dataKey="hour" stroke="#888" />
                <YAxis stroke="#888" />
                <Tooltip contentStyle={{ backgroundColor: '#222', border: 'none' }} />
                <Line type="monotone" dataKey="unique_visitors" stroke="#10b981" strokeWidth={3} dot={false} />
              </LineChart>
            </ResponsiveContainer>
          </div>
        </div>

        {/* Panel 2: Specific Domain Traffic (Count-Min Sketch) */}
        <div className="card">
          <div className="card-header-flex">
            <h2>Domain Traffic Spikes</h2>
            <select 
              value={selectedDomain} 
              onChange={(e) => setSelectedDomain(e.target.value)}
              className="domain-selector"
            >
              {uniqueDomainsList.map(domain => (
                <option key={domain} value={domain}>{domain}</option>
              ))}
            </select>
          </div>
          <div className="chart-container">
            <ResponsiveContainer width="100%" height="100%">
              <LineChart data={chartDataForDomain}>
                <CartesianGrid strokeDasharray="3 3" stroke="#333" />
                <XAxis dataKey="hour" stroke="#888" />
                <YAxis stroke="#888" />
                <Tooltip contentStyle={{ backgroundColor: '#222', border: 'none' }} />
                <Line type="stepAfter" dataKey="frequency" stroke="#3b82f6" strokeWidth={3} dot={false} />
              </LineChart>
            </ResponsiveContainer>
          </div>
        </div>

        {/* Panel 3: Suspicious IP Log (Bloom Filter) */}
        <div className="card full-width">
          <h2><ShieldAlert className="icon-small text-red" /> Security: Flagged IPs (Bloom Filter)</h2>
          <div className="table-container">
            <table>
              <thead>
                <tr>
                  <th>Detection Timestamp</th>
                  <th>Suspicious IP Address</th>
                  <th>Status</th>
                </tr>
              </thead>
              <tbody>
                {suspicious.map((flag, index) => (
                  <tr key={index}>
                    <td>{flag.timestamp}</td>
                    <td className="ip-text">{flag.ip_address}</td>
                    <td><span className="badge">Blocked</span></td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>
      </div>
    </div>
  );
}

export default App;