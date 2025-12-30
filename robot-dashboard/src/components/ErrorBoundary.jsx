import React from 'react';
import { AlertTriangle, RefreshCw } from 'lucide-react';

class ErrorBoundary extends React.Component {
  constructor(props) {
    super(props);
    this.state = { hasError: false, error: null, errorInfo: null };
  }

  static getDerivedStateFromError(error) {
    return { hasError: true };
  }

  componentDidCatch(error, errorInfo) {
    console.error('Dashboard Error:', error, errorInfo);
    this.setState({ error, errorInfo });
  }

  handleReset = () => {
    this.setState({ hasError: false, error: null, errorInfo: null });
    window.location.reload();
  };

  render() {
    if (this.state.hasError) {
      return (
        <div className="min-h-screen bg-gradient-to-br from-nightfall-primary via-nightfall-secondary to-nightfall-primary flex items-center justify-center p-6">
          <div className="card p-8 max-w-2xl w-full">
            <div className="text-center">
              <div className="inline-flex items-center justify-center w-20 h-20 bg-nightfall-error/20 rounded-full mb-6">
                <AlertTriangle className="w-10 h-10 text-nightfall-error" />
              </div>
              
              <h1 className="text-3xl font-bold text-white mb-3">
                Dashboard Error
              </h1>
              
              <p className="text-gray-400 mb-6">
                Something went wrong with the mission control dashboard. Don't worry, your robot is safe.
              </p>

              {this.state.error && (
                <div className="bg-gray-800 p-4 rounded-lg text-left mb-6 overflow-x-auto">
                  <p className="text-sm font-mono text-nightfall-error mb-2">
                    {this.state.error.toString()}
                  </p>
                  {this.state.errorInfo && (
                    <pre className="text-xs text-gray-500 max-h-40 overflow-y-auto">
                      {this.state.errorInfo.componentStack}
                    </pre>
                  )}
                </div>
              )}

              <button
                onClick={this.handleReset}
                className="btn-primary inline-flex items-center gap-2"
              >
                <RefreshCw className="w-5 h-5" />
                Reload Dashboard
              </button>

              <p className="text-sm text-gray-500 mt-6">
                If this problem persists, check browser console for details
              </p>
            </div>
          </div>
        </div>
      );
    }

    return this.props.children;
  }
}

export default ErrorBoundary;
